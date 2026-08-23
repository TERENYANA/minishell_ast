#!/bin/bash
# ============================================================================
# minishell_tester.sh — Strictly aligned with 42 subject
# ============================================================================

G='\033[32m'; R='\033[31m'; Y='\033[33m'; B='\033[34m'; DIM='\033[2m'; N='\033[0m'

MS="${1:-./minishell}"
SECTION="${2:-all}"
VERBOSE=0
DO_LEAKS=0
LEAKS_ONLY=0

for arg in "$@"; do
    case "$arg" in
        -v|--verbose) VERBOSE=1 ;;
        --leaks) DO_LEAKS=1 ;;
        --leaks-only) DO_LEAKS=1; LEAKS_ONLY=1 ;;
    esac
done

if [ ! -f "$MS" ] || [ ! -x "$MS" ] || [ -d "$MS" ]; then
    echo -e "${R}Erreur:${N} '$MS' n'est pas un executable."
    echo "Usage: $0 <path/minishell> [section] [-v] [--leaks|--leaks-only]"
    echo "Sections: parsing quotes redirects pipes env exit_status builtins"
    echo "          signals mix bonus_logic bonus_wildcards fd_advanced all"
    exit 1
fi
MS="$(cd "$(dirname "$MS")" && pwd)/$(basename "$MS")"

HAS_VG=0
command -v valgrind >/dev/null 2>&1 && HAS_VG=1
if [ "$DO_LEAKS" -eq 1 ] && [ "$HAS_VG" -eq 0 ]; then
    echo -e "${Y}Attention:${N} valgrind absent → leaks désactivés."
    DO_LEAKS=0
    [ "$LEAKS_ONLY" -eq 1 ] && exit 1
fi

VG_OPTS="--leak-check=full --show-leak-kind=all --errors-for-leak-kinds=all \
--trace-children=yes --track-fds=yes --error-exitcode=42"
[ -f "readline.supp" ] && VG_OPTS="$VG_OPTS --suppressions=readline.supp"

# ============================================================================
# ====================== MANDATORY ======================
# ============================================================================

parsing=(
    "echo hello"
    "echo hello world"
    "echo"
    "echo -n"
    "echo -n hello"
    "echo -nnn hello"
    "echo hello     world"
    "echo \"\""
    "echo ''"
    "/bin/echo hello"
    "ls"
    "ls -l"
    "pwd"
    "nonexistent_cmd_xyz"
    "./nonexistent"
    "/bin/nonexistent_xyz"
    "echo |"
    "| echo"
    "echo >"
    "> >"
    "> <"
    "> > >"
    "| |"
    "| | |"
    "cat |"
    "| cat"
    "echo \"unclosed"
)

quotes=(
    "echo 'hello world'"
    "echo \"hello world\""
    "echo 'hello'\"world\""
    "echo \"hello\"'world'"
    "echo '\$USER'"
    "echo \"\$USER\""
    "echo '\"\$USER\"'"
    "echo \"'\$USER'\""
    "echo \"hello ' world\""
    "echo 'hello \" world'"
    "echo \"42\"'\$'\"42\""
    "echo 'a'\"b\"'c'"
    "echo \"a\"'b'\"c\""
    "echo \"\"\"\""
    "echo ''''"
    "echo \"'\"'\"'\""
    "echo \$NONEXISTENT\$USER"
    "echo \"\" \"\" \"\""
    "echo '\$USER' \"\$USER\""
    "false | true"
    "true | false"
)

redirects=(
    "echo hello > out1"
    "echo hello >> out1"
    "cat < file1"
    "cat < file1 > out1"
    "cat < file1 >> out1"
    "echo a > out1"
    "echo b >> out1"
    "cat < file1 < file2"
    "echo 1 > out1 > out2"
    "echo 1 >> out1 >> out2"
    "cat > out1 < file1"
    "> out1"
    ">> out1"
    "cat < nonexistent"
    "echo hello > out1"
    "cat out1"
    "echo a > out1"
    "echo b > out1"
    "cat out1"
    "echo a > f1 > f2 > f3"
    "< nonexistent_file cat"
    "echo test > /does/not/exist/file"
    "cat << EOF | cat"
    "echo a > ."
    "echo a > /"
    "cat < ."
    "echo a > f1 > f2 > f3 > f4 > f5 > f6 > f7 > f8 > f9 > f10"
    "cat << \"EOF\" | cat"
    "cat << NONEXISTENT_DELIM"
)

pipes=(
    "echo hello | cat"
    "echo hello | cat | cat"
    "echo hello | cat | cat | cat"
    "ls | cat"
    "ls / | grep etc"
    "ls / | wc -l"
    "cat /etc/passwd | grep root | wc -l"
    "echo 1 | echo 2 | echo 3"
    "echo hi | cat | cat | echo done"
    "ls -l / | grep home | wc -l"
    "cat file1 | cat | cat > out1"
    "echo a | | echo b"
    "sleep 0.1 | sleep 0.1 | sleep 0.1"
    "echo a |"
    "| cat"
    "| | |"
)

env=(
    "echo \$USER"
    "echo \$HOME"
    "echo \$PATH"
    "echo \$PWD"
    "echo \$?"
    "echo \$EMPTY_VAR"
    "echo \"\$USER\""
    "echo '\$USER'"
    "echo \$USER\$HOME"
    "echo \"\$USER \$HOME\""
    "export TEST=hello"
    "echo \$TEST"
    "export A=1 B=2"
    "echo \$A \$B"
    "export TEST="
    "echo \"[\$TEST]\""
    "unset USER"
    "echo \$USER"
    "export TEST=42"
    "unset TEST"
    "echo \$TEST"
    "export =value"
    "export 1VAR=value"
    "unset 1VAR"
    "export VAR1=1 VAR2=2 VAR1=3"
    "echo \$?"
    "false
echo \$?"
    "export 1VAR=a"
    "export VAR-1=b"
    "export =c"
    "unset 1VAR VAR-1 ="
    "echo 'env | grep SHLVL' | ./minishell > /dev/null"
)

exit_status=(
    "echo 42"
    "true"
    "false"
    "nonexistent_cmd"
    "ls nonexistent_file"
    "cat < nonexistent"
    "true && false"
    "false || true"
    "exit"
    "exit 0"
    "exit 42"
    "exit 255"
    "exit 42 42"
    "exit hello"
    "exit +42"
    "exit -42"
    "exit 99999999999999999999"
)

builtins=(
    "echo hello"
    "echo -n hello"
    "echo -n -n hello"
    "pwd"
    "cd /"
    "pwd"
    "cd /tmp"
    "pwd"
    "cd .."
    "pwd"
    "cd ."
    "pwd"
    "cd /nonexistent"
    "cd"
    "export TEST=42"
    "env | grep TEST"
    "unset TEST"
    "env | grep TEST || echo gone"
    "env | head -n 5"
    "exit 0"
    "echo -n -n -nnnn -n hello"
    "export A=1 B=2 C=3"
    "unset A B C"
    "cd ~"
    "cd -"
)

signals=(
    "echo hello"
    "true"
    "false"
    "pwd"
)

mix=(
    "echo hello | cat > out1"
    "cat < file1 | cat > out1"
    "echo a > out1 | cat"
    "echo hi | cat | cat > out1"
    "cat < file1 | grep 1 > out1"
    "echo 1 | echo 2 > out1"
    "ls / | head -n 3 | cat > out1"
    "echo a > out1"
    "cat out1"
    "false || echo recovered > out1"
    "echo start | cat | cat > out1 | cat"
    "< file1 cat | wc -l > out1"
)

fd_advanced=(
    "cat < nonexistent | cat | cat > out1"
    "echo a | (cat < nonexistent) | cat"
    "echo test > f1 > f2 > f3 > f4 > f5"
    "< f1 < f2 < f3 < f4 < f5 cat"
    "echo start | cat | cat | cat | cat | cat | cat | cat | cat > out1"
    "(echo a | cat) | (cat | cat) > out1"
    "false || (echo a | cat | cat > out1)"
    "true && (cat < nonexistent_file | cat > out1)"
    "echo a > out1 | echo b > out2 | echo c > out3 | cat < out1 | cat < out2 | cat < out3"
    "cat << EOF | cat << EOF2 | cat << EOF3"
    "cat < file1 > out1 < file2 > out2 < file3 > out3"
    "echo a | | echo b"
    "echo a |"
    "> out1 > out2 > out3 > out4"
    "( ( ( echo a | cat ) | cat ) | cat ) > out1"
    "cd /tmp > /does/not/exist/file"
    "export TEST_FD=1 > out1 | unset TEST_FD"
    "< nonexistent echo a > out1"
    "echo a > out1 < nonexistent > out2"
    "echo a | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat | cat"
)

# ============================================================================
# ====================== BONUS ======================
# ============================================================================

bonus_logic=(
    "echo 42 && echo 21"
    "echo 42 || echo 21"
    "false && echo nope"
    "false || echo yes"
    "echo 1 && echo 2 && echo 3"
    "true && true && true"
    "false || false || true"
    "true && false || true"
    "false || true && false"
    "echo a && echo b || echo c"
    "echo a || echo b && echo c"
    "echo 1 && (echo 2 && echo 3)"
    "echo 1 && (echo 2 || echo 3) && echo 4"
    "(echo 1 && echo 2) || echo 3"
    "(echo 1 || echo 2) && (echo 3 || echo 4)"
    "echo 1 && (echo 2 || echo 3) || echo 4"
    "((echo 1 && echo 2) || (echo 3 && echo 4))"
    "true && (false || true)"
    "false || (true && false)"
    "()"
    "( ( (ls) ) )"
    "echo a && && echo b"
    "echo a || || echo b"
    "(echo a || echo b) | cat"
)

bonus_wildcards=(
    "echo *"
    "echo *.c"
    "echo file*"
    "ls *"
    "echo *1"
    "echo *2"
    "echo *1 *2"
    "echo nonexistent*pattern"
    "echo \"*\""
    "echo '*'"
    "ls file*"
    "echo * > out1"
    "echo * | cat"
    "echo */*"
    "echo s*/*"
)

# ============================================================================
# SANDBOX + RUNNERS (same as before)
# ============================================================================
WORK=$(mktemp -d /tmp/ms_test.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

setup_sandbox() {
    rm -rf "$1"; mkdir -p "$1"
    printf '1\n' > "$1/file1"
    printf '2\n' > "$1/file2"
    printf '3\n' > "$1/file3"
    printf 'hello\nworld\n' > "$1/data.txt"
    touch "$1/fileA" "$1/fileB" "$1/test.c" "$1/main.c" "$1/readme"
}

heredoc_input() {
    printf 'line one\nline two\nEOF\nEND\n'
}

TOTAL_PASS=0
TOTAL_FAIL=0
TOTAL_LEAK=0
TOTAL_FDLEAK=0

run_correctness() {
    local name="$1"; shift
    local tests=("$@")
    local pass=0 fail=0 i=0

    echo -e "\n${B}=== CORRECTION : $name ===${N}"

    for t in "${tests[@]}"; do
        i=$((i + 1))
        local Bdir="$WORK/b_${name}_$i" Mdir="$WORK/m_${name}_$i"
        setup_sandbox "$Bdir"; setup_sandbox "$Mdir"

        local feed
        if [[ "$t" == *"<<"* ]]; then
            feed="$(printf '%s\n' "$t"; heredoc_input)"
        else
            feed="$(printf '%s\n' "$t")"
        fi

        local bout brc
        bout=$( cd "$Bdir" && printf '%s' "$feed" | timeout 5 bash 2>/dev/null )
        setup_sandbox "$Bdir"
        brc=$( cd "$Bdir" && { printf '%s' "$feed" | timeout 5 bash >/dev/null 2>&1; echo $?; } )
        setup_sandbox "$Bdir"
        ( cd "$Bdir" && printf '%s' "$feed" | timeout 5 bash >/dev/null 2>&1 )

        local mraw mout mrc
        mraw=$( cd "$Mdir" && printf '%s' "$feed" | timeout 5 "$MS" 2>/dev/null )
        mout=$(printf '%s\n' "$mraw" \
            | sed -E 's/^minishell[$>#] ?//' \
            | sed -E 's/^\$ //' \
            | grep -v '^minishell' )
        mout=$(printf '%s' "$mout")
        setup_sandbox "$Mdir"
        mrc=$( cd "$Mdir" && { printf '%s' "$feed" | timeout 5 "$MS" >/dev/null 2>&1; echo $?; } )
        setup_sandbox "$Mdir"
        ( cd "$Mdir" && printf '%s' "$feed" | timeout 5 "$MS" >/dev/null 2>&1 )

        local norm='s#/tmp/ms_test\.[^/]*/[bm]_[a-z_]*_[0-9]*#SANDBOX#g'
        local filt='^(PWD=|OLDPWD=|SHLVL=|_=|LINES=|COLUMNS=|SHELL=)'
        bout=$(printf '%s\n' "$bout" | sed -E "$norm" | grep -vE "$filt")
        mout=$(printf '%s\n' "$mout" | sed -E "$norm" | grep -vE "$filt")

        if printf '%s' "$t" | grep -qE '(^|[^a-zA-Z_])env([^a-zA-Z_]|$)'; then
            bout=$(printf '%s\n' "$bout" | sort)
            mout=$(printf '%s\n' "$mout" | sort)
        fi

        local diffs=""
        [ "$mout" != "$bout" ] && diffs="stdout"
        [ "$mrc" != "$brc" ] && diffs="$diffs exit(bash=$brc/ms=$mrc)"
        for f in file1 file2 file3 out1 out2 out3 data.txt; do
            if [ -e "$Bdir/$f" ] || [ -e "$Mdir/$f" ]; then
                cmp -s "$Bdir/$f" "$Mdir/$f" 2>/dev/null || diffs="$diffs $f"
            fi
        done

        if [ -z "$diffs" ]; then
            printf "${G}[OK]${N} %2d: %s\n" "$i" "$t"
            pass=$((pass + 1))
        else
            printf "${R}[KO]${N} %2d: %s\n" "$i" "$t"
            printf "  ${DIM}diff:${N} %s\n" "$diffs"
            if [[ "$diffs" == *stdout* ]] || [ "$VERBOSE" -eq 1 ]; then
                printf "  ${DIM}bash:${N} [%s]\n" "$(printf '%s' "$bout" | head -5 | tr '\n' '|')"
                printf "  ${DIM}mini:${N} [%s]\n" "$(printf '%s' "$mout" | head -5 | tr '\n' '|')"
            fi
            fail=$((fail + 1))
        fi
    done

    echo -e "${DIM}  $name: $pass OK, $fail KO${N}"
    TOTAL_PASS=$((TOTAL_PASS + pass))
    TOTAL_FAIL=$((TOTAL_FAIL + fail))
}

run_leaks() {
    local name="$1"; shift
    local tests=("$@")
    local clean=0 leak=0 fdleak=0 i=0

    echo -e "\n${B}=== FUITES : $name ===${N}"

    for t in "${tests[@]}"; do
        i=$((i + 1))
        local Mdir="$WORK/vg_${name}_$i"
        setup_sandbox "$Mdir"

        local feed
        if [[ "$t" == *"<<"* ]]; then
            feed="$(printf '%s\n' "$t"; heredoc_input)"
        else
            feed="$(printf '%s\n' "$t")"
        fi

        local log="$Mdir/vg.log"
        ( cd "$Mdir" && printf '%s' "$feed" | timeout 20 valgrind $VG_OPTS \
            "$MS" >/dev/null 2>"$log" )

        local lost fds
        lost=$(grep -E "definitely lost|indirectly lost" "$log" 2>/dev/null \
            | grep -vE "0 bytes in 0 blocks" | head -5)
        fds=$(grep -A2 "FILE DESCRIPTORS" "$log" 2>/dev/null | grep -oE "[0-9]+ open" | head -1)

        local bad=""
        [ -n "$lost" ] && bad="leak"
        if [ -n "$fds" ]; then
            local nfd=$(echo "$fds" | grep -oE "^[0-9]+")
            [ "$nfd" -gt 3 ] 2>/dev/null && bad="$bad fd($nfd)"
        fi

        if [ -z "$bad" ]; then
            printf "${G}[CLEAN]${N} %2d: %s\n" "$i" "$t"
            clean=$((clean + 1))
        else
            printf "${R}[LEAK]${N} %2d: %s  ${DIM}(%s)${N}\n" "$i" "$t" "$bad"
            [[ "$bad" == *leak* ]] && leak=$((leak + 1))
            [[ "$bad" == *fd* ]] && fdleak=$((fdleak + 1))
            [ "$VERBOSE" -eq 1 ] && [ -n "$lost" ] && echo "$lost" | sed 's/^/    /'
        fi
    done

    echo -e "${DIM}  $name: $clean clean, $leak leaks, $fdleak fd-leaks${N}"
    TOTAL_LEAK=$((TOTAL_LEAK + leak))
    TOTAL_FDLEAK=$((TOTAL_FDLEAK + fdleak))
}

run_section() {
    local name="$1"
    local -n arr="$name"
    [ "$LEAKS_ONLY" -eq 0 ] && run_correctness "$name" "${arr[@]}"
    [ "$DO_LEAKS" -eq 1 ] && run_leaks "$name" "${arr[@]}"
}

SECTIONS_ALL="parsing quotes redirects pipes env exit_status builtins signals mix bonus_logic bonus_wildcards fd_advanced"

echo -e "${B}╔════════════════════════════════════════════╗${N}"
echo -e "${B}║  MINISHELL TESTER — Subject Strict         ║${N}"
echo -e "${B}╚════════════════════════════════════════════╝${N}"
echo -e "  binaire  : ${DIM}$MS${N}"
echo -e "  section  : ${DIM}$SECTION${N}"
[ "$DO_LEAKS" -eq 1 ] && echo -e "  valgrind : ${DIM}active${N}"

if [ "$SECTION" = "all" ]; then
    for s in $SECTIONS_ALL; do
        run_section "$s"
    done
else
    if echo "$SECTIONS_ALL" | grep -qw "$SECTION"; then
        run_section "$SECTION"
    else
        echo -e "${R}Section inconnue:${N} $SECTION"
        echo "Disponibles: $SECTIONS_ALL all"
        exit 1
    fi
fi

echo ""
echo -e "${B}════════════════ RESUME ════════════════${N}"
if [ "$LEAKS_ONLY" -eq 0 ]; then
    echo -e "  Correction : ${G}$TOTAL_PASS OK${N} / ${R}$TOTAL_FAIL KO${N}"
fi
if [ "$DO_LEAKS" -eq 1 ]; then
    echo -e "  Fuites mem : ${R}$TOTAL_LEAK${N}"
    echo -e "  Fuites fd  : ${R}$TOTAL_FDLEAK${N}"
fi
echo -e "${B}════════════════════════════════════════${N}"

[ "$TOTAL_FAIL" -gt 0 ] || [ "$TOTAL_LEAK" -gt 0 ] || [ "$TOTAL_FDLEAK" -gt 0 ] && exit 1
exit 0
