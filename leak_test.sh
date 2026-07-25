!/bin/bash
# ms_leak_test.sh - Minishell leak / crash / hang tester
#
# Design goals (learned the hard way):
#   1. valgrind --trace-children=yes is the only fully trustworthy option
#      for tests that fork() (pipes, subshells, external commands, heredocs).
#      Use it whenever available (i.e. on Linux, or an x86 Mac with a
#      working valgrind build).
#   2. macOS's `leaks --atExit --` cannot be trusted around fork(): it can
#      deadlock the child, producing fake hangs that are NOT bugs in your
#      code. So on macOS we only leak-check the tests that never fork
#      (builtins run alone at top level, pure syntax errors). Anything
#      that forks is still run — with a plain timeout, no leaks wrapper —
#      just to confirm it doesn't hang or crash. It won't tell you about
#      leaks in that path; for that you need Linux + valgrind.
#
# Usage: ./ms_leak_test.sh [-v] [-t timeout_seconds]
#   -v  verbose: show tool output for anything that fails
#   -t  per-test timeout in seconds (default 5)

VERBOSE=0
TIMEOUT_SECS=5

while getopts "vt:" opt; do
    case $opt in
        v) VERBOSE=1 ;;
        t) TIMEOUT_SECS="$OPTARG" ;;
    esac
done

if [ ! -x "./minishell" ]; then
    echo "Error: ./minishell not found or not executable. Build it first."
    exit 1
fi

# ---------------------------------------------------------------------------
# Tool detection
# ---------------------------------------------------------------------------
CHECKER=""
CAN_LEAK_CHECK_FORK=0

if command -v valgrind &> /dev/null; then
    CHECKER="valgrind"
    LEAK_CMD=(valgrind --leak-check=full --show-leak-kinds=all \
              --errors-for-leak-kinds=all --trace-children=yes \
              --error-exitcode=42 -q)
    CAN_LEAK_CHECK_FORK=1
elif command -v leaks &> /dev/null; then
    CHECKER="leaks"
    LEAK_CMD=(leaks --atExit --)
    CAN_LEAK_CHECK_FORK=0
else
    echo "Error: Neither valgrind nor leaks command found."
    exit 1
fi

if command -v timeout &> /dev/null; then
    TIMEOUT_BIN="timeout"
elif command -v gtimeout &> /dev/null; then
    TIMEOUT_BIN="gtimeout"
else
    TIMEOUT_BIN=""
fi

echo "=========================================="
echo "  MINISHELL LEAK / CRASH / HANG TESTER"
echo "  checker: $CHECKER  |  timeout: ${TIMEOUT_SECS}s"
if [ "$CAN_LEAK_CHECK_FORK" -eq 0 ]; then
    echo "  NOTE: '$CHECKER' can't safely trace fork()."
    echo "  Forking tests run correctness-only (no leak data)."
    echo "  For full leak coverage of those, use Linux + valgrind."
fi
echo "=========================================="

# ---------------------------------------------------------------------------
# Test set A: never forks (pure syntax errors, or bare builtins running
# alone/chained at top level via run_builtin_in_parent). Safe to leak-check
# with either tool.
# ---------------------------------------------------------------------------
NOFORK_TESTS=(
    "echo hello |"
    "echo hello ||"
    "echo hello &&"
    "("
    ")"
    "((echo hello)"
    "(echo hello))"
    "echo hello >"
    "echo hello >>"
    "echo hello <"
    "echo a ||| echo b"
    "echo a &&& echo b"
    "| echo hello"
    "&& echo hello"
    "|| echo hello"
    "()"
    "( )"
    "echo 'unclosed"
    "echo \"unclosed"
    "echo \$"
    "echo \${"
    ""
    "   "
    "      |      "
    "echo hi"
    "echo -n hi"
    "echo \$?"
    "echo \$UNSET_VAR_XYZ"
    "echo \"\$UNSET_VAR_XYZ\""
    "echo 'a'\"b\"'c'"
    "echo \"it's a test\""
    "echo *.nonexistentext_xyz"
"cd /tmp
cd /
pwd"
"cd /tmp
cd -
cd -
pwd"
"cd
cd /tmp
cd ~
pwd"
"export FOO=bar
export FOO=baz
export FOO=qux
unset FOO"
"export A=1 B=2 C=3
export A=one
unset A B C"
"export
export X=1
export
unset X
export"
"export INVALID-NAME=oops
export 123bad=oops
export =oops"
"export --
export -- Y=1
unset Y"
"cd /tmp
export TESTVAR=hello
cd -
export TESTVAR=world
unset TESTVAR
pwd"
"cd nonexistent_dir_xyz
pwd"
"unset
unset -x
unset PATH HOME"
)

# ---------------------------------------------------------------------------
# Test set B: forks (pipes, subshells, external binaries, heredocs).
# Full leak coverage only when CAN_LEAK_CHECK_FORK=1.
# ---------------------------------------------------------------------------
FORK_TESTS=(
    "cat file_does_not_exist_xyz.txt"
    "cat '' '' '' '' ''"
    "cat < \"\""
    "echo 1 | cat"
    "echo 1 | cat | cat | cat"
    "echo 1 | echo 2 | echo 3 | echo 4 | echo 5"
    "cat file.txt | grep a |"
    "(echo a && echo b) |"
    "(echo 1 | cat) && (echo 2 | cat)"
    "((((echo 1) && echo 2) || echo 3) | cat)"
    "(echo hi) > /tmp/ms_leak_out1.txt"
    "(echo a; echo b) >> /tmp/ms_leak_out1.txt"
    "(cat) < /tmp/ms_leak_out1.txt"
    "((echo a) > /tmp/ms_leak_out1.txt) && ((echo b) >> /tmp/ms_leak_out1.txt)"
    "cat << EOF"
    "cat << 'EOF'"
    "cat << EOF | grep a"
    "cat << EOF << EOF2"
    "echo a << EOF && echo b"
    "(cat << EOF) | cat"
    "cd /tmp
(pwd) > /tmp/ms_leak_out1.txt
export SESSVAR=1
(echo \$SESSVAR | cat) >> /tmp/ms_leak_out1.txt
cd -
unset SESSVAR"
    "/bin/echo external_bin_test"
    "nonexistent_command_xyz123"
    "/nonexistent/path/to/nothing"
)

# ---------------------------------------------------------------------------
# Execution
# ---------------------------------------------------------------------------
PASSED=0
FAILED_LEAK=0
FAILED_CRASH=0
FAILED_HANG=0
SKIPPED_LEAK_CHECK=0

run_with_tool() {
    local test_input="$1"
    local out ret
    if [ -n "$TIMEOUT_BIN" ]; then
        out=$(printf '%s\n' "$test_input" | "$TIMEOUT_BIN" "$TIMEOUT_SECS" "${LEAK_CMD[@]}" ./minishell 2>&1)
        ret=$?
    else
        out=$(printf '%s\n' "$test_input" | "${LEAK_CMD[@]}" ./minishell 2>&1)
        ret=$?
    fi
    echo "$ret|$out"
}

run_plain() {
    local test_input="$1"
    local out ret
    if [ -n "$TIMEOUT_BIN" ]; then
        out=$(printf '%s\n' "$test_input" | "$TIMEOUT_BIN" "$TIMEOUT_SECS" ./minishell 2>&1)
        ret=$?
    else
        out=$(printf '%s\n' "$test_input" | ./minishell 2>&1)
        ret=$?
    fi
    echo "$ret|$out"
}

classify() {
    local ret="$1" out="$2"
    STATUS=""
    if [ -n "$TIMEOUT_BIN" ] && [ "$ret" -eq 124 ]; then
        STATUS="HANG"
    elif [ "$CHECKER" = "valgrind" ] && [ "$ret" -eq 42 ]; then
        STATUS="LEAK"
    elif [ "$CHECKER" = "leaks" ] && echo "$out" | grep -qi "leaks for"; then
        local leak_line leak_count
        leak_line=$(echo "$out" | grep -Eo '[0-9]+ leaks for [0-9]+ total leaked bytes' | tail -1)
        leak_count=$(echo "$leak_line" | grep -Eo '^[0-9]+')
        if [ -n "$leak_count" ] && [ "$leak_count" != "0" ]; then
            STATUS="LEAK"
        fi
    elif [ "$ret" -ge 128 ] || [ "$ret" -eq 139 ] || [ "$ret" -eq 134 ] || [ "$ret" -eq 136 ]; then
        STATUS="CRASH"
    fi
}

report() {
    local out="$1"
    case "$STATUS" in
        "")
            echo -e "\033[32m[ OK ]\033[0m"
            ((PASSED++))
            ;;
        LEAK)
            echo -e "\033[31m[ LEAK/DOUBLE-FREE ]\033[0m"
            ((FAILED_LEAK++))
            [ $VERBOSE -eq 1 ] && echo "$out" | sed 's/^/    /'
            ;;
        CRASH)
            echo -e "\033[35m[ CRASH (exit $RET) ]\033[0m"
            ((FAILED_CRASH++))
            [ $VERBOSE -eq 1 ] && echo "$out" | sed 's/^/    /'
            ;;
        HANG)
            echo -e "\033[33m[ TIMEOUT / HANG ]\033[0m"
            ((FAILED_HANG++))
            [ $VERBOSE -eq 1 ] && echo "$out" | sed 's/^/    /'
            ;;
    esac
}

echo ""
echo "--- Set A: no-fork tests (full leak check) ---"
i=0
for t in "${NOFORK_TESTS[@]}"; do
    i=$((i + 1))
    label=$(echo "$t" | tr '\n' ';' | cut -c1-55)
    printf 'A%-3d "%s" ... ' "$i" "$label"
    result=$(run_with_tool "$t")
    RET="${result%%|*}"
    OUT="${result#*|}"
    classify "$RET" "$OUT"
    report "$OUT"
done

echo ""
echo "--- Set B: forking tests ---"
i=0
for t in "${FORK_TESTS[@]}"; do
    i=$((i + 1))
    label=$(echo "$t" | tr '\n' ';' | cut -c1-55)
    printf 'B%-3d "%s" ... ' "$i" "$label"
    if [ "$CAN_LEAK_CHECK_FORK" -eq 1 ]; then
        result=$(run_with_tool "$t")
        RET="${result%%|*}"
        OUT="${result#*|}"
        classify "$RET" "$OUT"
        report "$OUT"
    else
        result=$(run_plain "$t")
        RET="${result%%|*}"
        OUT="${result#*|}"
        STATUS=""
        if [ -n "$TIMEOUT_BIN" ] && [ "$RET" -eq 124 ]; then
            STATUS="HANG"
        elif [ "$RET" -ge 128 ] || [ "$RET" -eq 139 ] || [ "$RET" -eq 134 ] || [ "$RET" -eq 136 ]; then
            STATUS="CRASH"
        fi
        if [ -z "$STATUS" ]; then
            echo -e "\033[36m[ OK, leak-check SKIPPED ]\033[0m"
            ((PASSED++))
            ((SKIPPED_LEAK_CHECK++))
        else
            report "$OUT"
        fi
    fi
done

rm -f /tmp/ms_leak_out1.txt

TOTAL=$((PASSED + FAILED_LEAK + FAILED_CRASH + FAILED_HANG))
echo ""
echo "=========================================="
echo "Results: $PASSED/$TOTAL OK, $FAILED_LEAK leaks, $FAILED_CRASH crashes, $FAILED_HANG hangs"
if [ "$SKIPPED_LEAK_CHECK" -gt 0 ]; then
    echo "($SKIPPED_LEAK_CHECK of the OK results were correctness-only, no leak data — $CHECKER can't trace fork() safely)"
fi
echo "=========================================="

if [ $((FAILED_LEAK + FAILED_CRASH + FAILED_HANG)) -gt 0 ]; then
    exit 1
fi
exit 0