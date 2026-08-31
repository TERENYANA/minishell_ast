#!/bin/bash
# ============================================================================
# leak_tester.sh — comprehensive valgrind leak/fd tester for minishell
# ============================================================================
# Usage: ./leak_tester.sh [path/to/minishell] [-v]
#   -v : show full valgrind output for FAILED cases (default: summary only)
#
# For each test case, feeds the given commands to minishell via stdin under
# valgrind, then checks:
#   - definitely lost / indirectly lost  == 0 bytes  (hard leak -> FAIL)
#   - possibly lost                       == 0 bytes  (FAIL, but flagged separately)
#   - no "Invalid free" / "Invalid read" / "Invalid write" (memory corruption -> FAIL)
#   - no self-owned fd left open at exit that isn't 0/1/2 or inherited
#     (best-effort: relies on --track-fds output, ignores /dev/ptmx <inherited>)
# "still reachable" is reported but does NOT fail the test (readline/libc noise).
# ============================================================================

G='\033[32m'; R='\033[31m'; Y='\033[33m'; B='\033[34m'; DIM='\033[2m'; N='\033[0m'

MS="${1:-./minishell}"
VERBOSE=0
[ "$2" = "-v" ] && VERBOSE=1
[ "$1" = "-v" ] && { VERBOSE=1; MS="./minishell"; }

if [ ! -f "$MS" ] || [ ! -x "$MS" ]; then
    echo -e "${R}Erreur:${N} '$MS' n'est pas un executable."
    echo "Usage: $0 <path/minishell> [-v]"
    exit 1
fi
MS="$(cd "$(dirname "$MS")" && pwd)/$(basename "$MS")"

command -v valgrind >/dev/null 2>&1 || { echo -e "${R}valgrind absent.${N}"; exit 1; }

SUPP=""
[ -f "readline.supp" ] && SUPP="--suppressions=readline.supp"

WORK=$(mktemp -d /tmp/leak_test.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

TOTAL=0
PASS=0
FAIL=0
FAIL_NAMES=()

# ----------------------------------------------------------------------------
# Test case definitions: name -> multi-line command script (fed via stdin)
# ----------------------------------------------------------------------------
declare -A CASES

CASES["simple_cmd"]='echo hello world
exit 0'

CASES["pwd_cd"]='pwd
cd /tmp
pwd
cd -
pwd
exit 0'

CASES["export_env_unset"]='export FOO=bar
echo $FOO
unset FOO
env | grep FOO
export A=1 B=2 C=3
unset A B C
exit 0'

CASES["export_append"]='export HOLA=bonjour
export HOLA+=world
echo $HOLA
exit 0'

CASES["pipes_simple"]='echo hi | cat | wc -l
exit 0'

CASES["pipes_long"]='echo a | cat | cat | cat | cat | wc -l
exit 0'

CASES["redirects_out_in"]="echo a > $WORK/out.txt
cat < $WORK/out.txt
echo append >> $WORK/out.txt
cat $WORK/out.txt
exit 0"

CASES["redirects_multi"]="echo a > $WORK/f1 > $WORK/f2 > $WORK/f3
cat $WORK/f3
exit 0"

CASES["logic_and_or"]='true && echo yes
false || echo fallback
false && echo nope
true || echo skip
exit 0'

CASES["subshell"]='(echo sub1 && echo sub2) | cat
(echo nested)
exit 0'

CASES["subshell_nested"]='( ( echo a && echo b ) | cat )
exit 0'

CASES["heredoc_basic"]='cat << EOF
line one
line two
EOF
exit 0'

CASES["heredoc_expand"]='export FOO=bar
cat << EOF
value is $FOO
EOF
exit 0'

CASES["heredoc_quoted"]="cat << 'EOF'
no \$expand here
EOF
exit 0"

CASES["error_cmd_not_found"]='nonexistent_command_xyz
ls nonexistent_file_xyz
exit 0'

CASES["error_syntax"]='| bad syntax
&& also bad
exit 0'

CASES["wildcard"]='echo *
echo *.c
exit 0'

CASES["exit_root_builtin"]='echo before
exit 0'

CASES["exit_and_chain"]='true && exit 5'

CASES["exit_or_chain"]='false || exit 2'

CASES["exit_pipe"]='echo a | exit 3'

CASES["exit_subshell"]='(exit 7)'

CASES["exit_arg_variants"]='exit 42
exit -42
exit abc
exit 1 2'

CASES["quotes_mixed"]='echo "hello world"
echo '"'"'single quoted'"'"'
echo "mix"'"'"'d'"'"'"quotes"
exit 0'

CASES["expand_vars"]='export FOO=bar
echo $FOO
echo "$FOO and ${FOO}"
echo $UNDEFINED_VAR_XYZ
exit 0'

CASES["multi_pipe_redirect"]="echo hello | cat | cat > $WORK/multi.txt
cat $WORK/multi.txt
exit 0"

CASES["cd_edgecases"]='cd
cd ..
cd .
cd -
cd /nonexistent_dir_xyz
exit 0'

CASES["empty_and_whitespace"]='

   
echo after empties
exit 0'

CASES["and_or_pipe_combo"]='true && echo a | cat
false || echo b | cat
exit 0'

# ----------------------------------------------------------------------------
# Runner
# ----------------------------------------------------------------------------

run_case() {
    local name="$1"
    local script="$2"
    local log="$WORK/${name}.log"

    printf '%s\n' "$script" \
        | valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes \
            --error-exitcode=97 $SUPP "$MS" >"$WORK/${name}.stdout" 2>"$log"

    local ok=1
    local reasons=()

    # Hard leaks: definitely/indirectly lost must be 0 in EVERY block
    if grep -qE 'definitely lost: [1-9]' "$log"; then
        ok=0
        reasons+=("definitely lost > 0")
    fi
    if grep -qE 'indirectly lost: [1-9]' "$log"; then
        ok=0
        reasons+=("indirectly lost > 0")
    fi
    if grep -qE 'possibly lost: [1-9]' "$log"; then
        ok=0
        reasons+=("possibly lost > 0")
    fi

    # Memory corruption / invalid ops
    if grep -qE 'Invalid (free|read|write)\(\)' "$log"; then
        ok=0
        reasons+=("invalid memory access")
    fi

    # fd leaks: a self-opened fd still open at exit, with a stack trace
    # pointing into the minishell binary (not libc/readline internals),
    # and not the inherited /dev/ptmx line.
    if grep -A3 'Open file descriptor' "$log" \
        | grep -q "$(basename "$MS")"; then
        ok=0
        reasons+=("fd leak (self-opened, not closed)")
    fi

    TOTAL=$((TOTAL + 1))
    if [ "$ok" -eq 1 ]; then
        printf "${G}[PASS]${N} %-28s\n" "$name"
        PASS=$((PASS + 1))
    else
        printf "${R}[FAIL]${N} %-28s ${DIM}%s${N}\n" "$name" "${reasons[*]}"
        FAIL=$((FAIL + 1))
        FAIL_NAMES+=("$name")
        if [ "$VERBOSE" -eq 1 ]; then
            echo -e "${DIM}--- full valgrind log: $log ---${N}"
            cat "$log"
            echo -e "${DIM}--- end log ---${N}"
        fi
    fi
}

echo -e "${B}╔══════════════════════════════════════════════╗${N}"
echo -e "${B}║  MINISHELL LEAK TESTER (valgrind)            ║${N}"
echo -e "${B}╚══════════════════════════════════════════════╝${N}"
echo -e "  binaire : ${DIM}$MS${N}"
echo -e "  suppr.  : ${DIM}${SUPP:-none}${N}"
echo ""

SORTED_NAMES=($(printf '%s\n' "${!CASES[@]}" | sort))
for name in "${SORTED_NAMES[@]}"; do
    run_case "$name" "${CASES[$name]}"
done

echo ""
echo -e "${B}════════════════ RESUME ════════════════${N}"
echo -e "  ${G}$PASS PASS${N} / ${R}$FAIL FAIL${N} / $TOTAL total"
if [ "$FAIL" -gt 0 ]; then
    echo -e "  ${R}Failed:${N} ${FAIL_NAMES[*]}"
    echo -e "  ${DIM}Logs kept in: $WORK (until this shell trap fires)${N}"
    echo -e "  ${DIM}Rerun with -v to see full valgrind output inline.${N}"
fi
echo -e "${B}══════════════════════════════════════════${N}"

[ "$FAIL" -gt 0 ] && exit 1
exit 0