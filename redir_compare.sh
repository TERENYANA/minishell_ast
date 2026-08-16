#!/bin/bash
# redir_compare.sh — compare minishell vs bash on the tester's redirect suite.
# For each test: fresh sandbox dirs (identical file1..file4), run both shells,
# diff stdout, exit code, and the contents of every file in the sandbox.
# Usage: ./redir_compare.sh /path/to/minishell

MS="${1:-./minishell}"
if [ ! -x "$MS" ]; then
    echo "Usage: $0 /path/to/minishell"; exit 1
fi
MS="$(cd "$(dirname "$MS")" && pwd)/$(basename "$MS")"

TESTS=(
    "< file1"
    "> file1"
    ">> file1"
    "< file1 > file2"
    "< file1 >> file2"
    "< file1 > file2 > file3 >> file4"
    "> file1 > file2 > file3 > file4"
    "cat < file1"
    "cat < file1 > file2 > file3"
    "cat < file1 >> file2 >> file3"
    "cat < file1 >> file2 > file3"
    "cat < file1 > file2 >> file3"
    "cat < file1 < file2"
    "cat < file1 < file2 < file3 < file4"
    "cat < file1 < file2 > file3 >> file4"
    "cat < file1 < file2 > file3 > file4"
    "cat < file1 >> file2 >> file3 < file4"
    "cat < file1 > file2 < file3"
    "cat > file1 < file2"
    "cat >> file1 < file2 < file3"
    "cat >> file1 > file2 < file3 < file4"
    "cat >> file1 >> file2 < file3 < file4"
    "cat >> file1 >> file2 >> file3 < file4"
    "cat /etc/passwd < /etc/passwd > file1"
    "cat /etc/passwd < /etc/passwd > file1 > file2"
    "cat /etc/passwd < /etc/passwd > file1 > file2 > file3"
    "cat file1 < /etc/passwd > file1 > file2 > file3"
    "cat file1 < /etc/passwd > file1 > file2 > file3 > file4"
    "cat file1 < /etc/passwd >> file1 >> file2"
    "cat file1 < /etc/passwd >> file1 >> file2 >> file3 >> file4"
)

WORK=$(mktemp -d /tmp/redir_cmp.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

setup_sandbox() {
    rm -rf "$1"; mkdir -p "$1"
    printf 'content of file1\n' > "$1/file1"
    printf 'content of file2\n' > "$1/file2"
    printf 'content of file3\n' > "$1/file3"
    printf 'content of file4\n' > "$1/file4"
}

PASS=0; FAIL=0
i=0
for t in "${TESTS[@]}"; do
    i=$((i + 1))
    B="$WORK/bash_$i"; M="$WORK/ms_$i"
    setup_sandbox "$B"; setup_sandbox "$M"

    bout=$( cd "$B" && printf '%s\n' "$t" | timeout 5 bash 2>/dev/null ); brc=$?
    mout=$( cd "$M" && printf '%s\n' "$t" | timeout 5 "$MS" 2>/dev/null ); mrc=$?

    # Strip readline echo/prompt lines if minishell echoes input in pipe mode:
    # remove any line that contains the prompt or equals the test itself.
    mclean=$(printf '%s\n' "$mout" | grep -vF "minishell\$" | grep -vxF "$t")
    bclean="$bout"

    diffs=""
    [ "$mclean" != "$bclean" ] && diffs="stdout"
    [ "$mrc" != "$brc" ] && diffs="$diffs exit($brc vs $mrc)"
    for f in file1 file2 file3 file4; do
        if ! cmp -s "$B/$f" "$M/$f" 2>/dev/null; then
            if [ -e "$B/$f" ] || [ -e "$M/$f" ]; then
                diffs="$diffs $f"
            fi
        fi
        if [ -e "$B/$f" ] && [ ! -e "$M/$f" ]; then diffs="$diffs $f(missing)"; fi
        if [ ! -e "$B/$f" ] && [ -e "$M/$f" ]; then diffs="$diffs $f(extra)"; fi
    done

    if [ -z "$diffs" ]; then
        printf '\033[32m[OK]\033[0m  %s\n' "$t"
        ((PASS++))
    else
        printf '\033[31m[KO]\033[0m  %s\n' "$t"
        printf '      differs: %s\n' "$diffs"
        if [ "$mclean" != "$bclean" ]; then
            printf '      bash stdout: [%s]\n' "$(printf '%s' "$bclean" | head -3 | tr '\n' '|')"
            printf '      mini stdout: [%s]\n' "$(printf '%s' "$mclean" | head -3 | tr '\n' '|')"
        fi
        for f in file1 file2 file3 file4; do
            if ! cmp -s "$B/$f" "$M/$f" 2>/dev/null; then
                printf '      %s bash: [%s]\n' "$f" "$(head -c 60 "$B/$f" 2>/dev/null | tr '\n' '|')"
                printf '      %s mini: [%s]\n' "$f" "$(head -c 60 "$M/$f" 2>/dev/null | tr '\n' '|')"
            fi
        done
        ((FAIL++))
    fi
done

echo "=============================="
echo "PASS: $PASS / $((PASS + FAIL))"
[ $FAIL -gt 0 ] && exit 1 || exit 0