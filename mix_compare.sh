#!/bin/bash
# mix_compare.sh — compare minishell vs bash on the tester's mix_mandatory suite.
# These tests combine pipes AND file redirections, so this script merges both
# proven approaches: fresh identical sandboxes (file1-4 pre-created) for each
# shell + direct-pipe protocol + comparison of stdout, exit code, and files.
# Usage: ./mix_compare.sh /path/to/minishell [-v]

MS="${1:-./minishell}"
VERBOSE=0
[ "$2" = "-v" ] && VERBOSE=1

if [ ! -f "$MS" ] || [ ! -x "$MS" ] || [ -d "$MS" ]; then
    echo "Error: '$MS' is not an executable file."
    echo "Usage: $0 /path/to/minishell [-v]"
    exit 1
fi
MS="$(cd "$(dirname "$MS")" && pwd)/$(basename "$MS")"

TESTS=(
    'ls -al -i < /etc/passwd'
    'ls -al -i < /etc/passwd > file1'
    'ls -al -i < /etc/passwd > file1 > file2'
    'ls -al -i < /etc/passwd > file1 > file2 > file3'
    'ls -al -i < /etc/passwd > file1 > file2 > file3 > file4'
    'wc -l < /etc/passwd'
    'wc -l < /etc/passwd > file1 | wc -l'
    'wc -l < /etc/passwd > file1 > file2 > file3 > file4'
    'ls -al -i < /etc/passwd > file1 | wc -l | echo 42'
    'cat < file1 | cat > file2'
    'cat < file1 | grep 1 > file2 | wc -l'
    'cat < file1 | grep 1 > file2 | wc -l | echo 42'
    'cat < file1 | grep 1 > file2 | wc -l | echo 42 | echo 21'
    '> file1 | echo 42 | cat < file1 | grep 1 > file2 | wc -l'
    '< file1 | echo 42 > file2 | grep 1 | wc -l'
    'cat < file1 | cat > file2 < file1 | wc -l'
    'echo 42 > file1 | cat | wc -l'
    'echo 42 < file1 | cat | wc -l'
    '>> file1 | cat'
    '>> file1 | echo 42'
    '> file1 | cat < file1'
    '> file1 | echo 42 < file1 | cat file1'
    'echo 42 | wc -l | cat > file1'
    'echo 42 | cat | cat | cat | cat'
    'echo 42 | cat | cat | cat | cat | echo 21'
    'cat < file1 >> file2 > file1 | cat'
    'cat < file1 >> file2 > file1 | cat | echo 21'
    'ls / | grep c | cat > file1'
    'ls / | grep home | wc -l'
    'ls / | grep home | wc -l | < file1 > file2 | cat'
)

WORK=$(mktemp -d /tmp/mix_cmp.XXXXXX)
trap 'rm -rf "$WORK"' EXIT

setup_sandbox() {
    rm -rf "$1"; mkdir -p "$1"
    printf '1\n' > "$1/file1"
    printf '2\n' > "$1/file2"
    printf '3\n' > "$1/file3"
    printf '4\n' > "$1/file4"
}

PASS=0
FAIL=0
i=0

for t in "${TESTS[@]}"; do
    i=$((i + 1))
    B="$WORK/bash_$i"; M="$WORK/ms_$i"
    setup_sandbox "$B"; setup_sandbox "$M"

    # --- bash run (stdout, then exit code in a second identical run) ---
    bout=$( cd "$B" && printf '%s\n' "$t" | timeout 5 bash 2>/dev/null )
    setup_sandbox "$B"
    brc=$( cd "$B" && { printf '%s\n' "$t" | timeout 5 bash >/dev/null 2>&1; echo $?; } )
    # NOTE: the exit-code run also re-modified files; re-run once more for
    # final file state so file comparison reflects a single clean execution.
    setup_sandbox "$B"
    ( cd "$B" && printf '%s\n' "$t" | timeout 5 bash >/dev/null 2>&1 )

    # --- minishell run (same three-phase approach) ---
    mraw=$( cd "$M" && printf '%s\n' "$t" | timeout 5 "$MS" 2>/dev/null )
    mout=$(printf '%s\n' "$mraw" | grep -v '^minishell\$')
    mout=$(printf '%s' "$mout")
    setup_sandbox "$M"
    mrc=$( cd "$M" && { printf '%s\n' "$t" | timeout 5 "$MS" >/dev/null 2>&1; echo $?; } )
    setup_sandbox "$M"
    ( cd "$M" && printf '%s\n' "$t" | timeout 5 "$MS" >/dev/null 2>&1 )

    diffs=""
    [ "$mout" != "$bout" ] && diffs="stdout"
    [ "$mrc" != "$brc" ] && diffs="$diffs exit(bash=$brc ms=$mrc)"
    for f in file1 file2 file3 file4; do
        if ! cmp -s "$B/$f" "$M/$f" 2>/dev/null; then
            diffs="$diffs $f"
        fi
    done

    if [ -z "$diffs" ]; then
        printf '\033[32m[OK]\033[0m  %2d: %s\n' "$i" "$t"
        ((PASS++))
    else
        printf '\033[31m[KO]\033[0m  %2d: %s\n' "$i" "$t"
        printf '       differs: %s\n' "$diffs"
        if [[ "$diffs" == *stdout* ]] || [ $VERBOSE -eq 1 ]; then
            printf '       bash out: [%s]\n' "$(printf '%s' "$bout" | head -3 | tr '\n' '|')"
            printf '       mini out: [%s]\n' "$(printf '%s' "$mout" | head -3 | tr '\n' '|')"
        fi
        for f in file1 file2 file3 file4; do
            if ! cmp -s "$B/$f" "$M/$f" 2>/dev/null; then
                printf '       %s bash: [%s]\n' "$f" "$(head -c 50 "$B/$f" 2>/dev/null | tr '\n' '|')"
                printf '       %s mini: [%s]\n' "$f" "$(head -c 50 "$M/$f" 2>/dev/null | tr '\n' '|')"
            fi
        done
        ((FAIL++))
    fi
done

echo "=============================="
echo "PASS: $PASS / $((PASS + FAIL))"
[ $FAIL -gt 0 ] && exit 1 || exit 0