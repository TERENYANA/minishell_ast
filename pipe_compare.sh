#!/bin/bash
# pipe_compare.sh — compare minishell vs bash on the tester's pipes suite.
# Uses the direct-pipe protocol (printf | shell), which we verified works
# reliably with this minishell — no pty, no races, no single-read gambling.
# Compares: stdout (readline echo/prompt stripped) and exit code ($?).
# Usage: ./pipe_compare.sh /path/to/minishell [-v]

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
    'ls / | grep etc'
    'ls / | grep etc | echo 42'
    'ls / | wc -l'
    'ls / | head -n 5'
    'ls -l / | grep home'
    'ls -l / | grep home | wc -l'
    'cat /etc/passwd | grep bash'
    'cat /etc/passwd | grep bash | wc -l'
    'echo "Hello, World!" | wc -w'
    'echo "Hello, World!" | sed '"'"'s/World/Universe/g'"'"''
    'echo "Hello, World!" | cut -d " " -f 4'
    'echo "Hello, World!" | cut -d " " -f 4-6'
    'echo "Hello, World!" | cut -c 5-15'
    'ps -ef | grep apache | grep -v grep'
    'echo 1 | echo 2 | echo 3 | echo 4 | echo 5 | echo 6 | echo 7'
    'echo 1 | echo 2 | echo 3 | echo 4 | echo 42 | grep 2'
    'echo $SHELL | grep bash'
    'echo $LOGNAME | grep $LOGNAME'
    'echo "$USER" 42 "'"'"'$SHELL'"'"'" | grep bash | wc -l'
    'echo "$USER" 42 "'"'"'$SHELL'"'"'" | grep bash | wc -l | echo 42'
    'echo "$USER" 42 "'"'"'$SHELL'"'"'" | echo 42'
    'ls / | grep etc | echo 42 | ls / | grep etc | echo 42 | echo 21'
    'ls -al / | grep e | wc -l'
    'ls -al / | grep e | wc -l | echo 42'
    'ls -a -l / | wc -l'
    'echo '"'"'$USER'"'"' | grep $USER'
    'echo '"'"'$USER'"'"' | grep $USER | echo 42'
    'echo "'"'"'$SHELL'"'"'" | grep bash'
    'echo "'"'"'$SHELL'"'"'" | grep bash | grep b'
    'echo '"'"''"'"'$USER'"'"''"'"' | grep $USER'
)

PASS=0
FAIL=0
i=0

for t in "${TESTS[@]}"; do
    i=$((i + 1))

    # --- bash: fresh process, command + exit status in one session
    bout=$(printf '%s\n' "$t" | timeout 5 bash 2>/dev/null)
    brc=$( { printf '%s\n' "$t" | timeout 5 bash >/dev/null 2>&1; echo $?; } )

    # --- minishell: same, then strip readline echo lines and prompts.
    # In pipe mode readline echoes "minishell$ <command>" lines; drop any
    # line starting with the prompt, and a possible bare trailing prompt.
    mraw=$(printf '%s\n' "$t" | timeout 5 "$MS" 2>/dev/null)
    mout=$(printf '%s\n' "$mraw" | grep -v '^minishell\$' | sed 's/^minishell\$ *$//')
    # remove possible trailing empty line created by stripping the prompt
    mout=$(printf '%s' "$mout")
    mrc=$( { printf '%s\n' "$t" | timeout 5 "$MS" >/dev/null 2>&1; echo $?; } )

    status=""
    [ "$mout" != "$bout" ] && status="stdout"
    [ "$mrc" != "$brc" ] && status="$status exit(bash=$brc ms=$mrc)"

    if [ -z "$status" ]; then
        printf '\033[32m[OK]\033[0m  %2d: %s\n' "$i" "$t"
        ((PASS++))
    else
        printf '\033[31m[KO]\033[0m  %2d: %s\n' "$i" "$t"
        printf '       differs: %s\n' "$status"
        if [ "$mout" != "$bout" ] || [ $VERBOSE -eq 1 ]; then
            printf '       bash: [%s]\n' "$(printf '%s' "$bout" | head -5 | tr '\n' '|')"
            printf '       mini: [%s]\n' "$(printf '%s' "$mout" | head -5 | tr '\n' '|')"
        fi
        ((FAIL++))
    fi
done

echo "=============================="
echo "PASS: $PASS / $((PASS + FAIL))"
[ $FAIL -gt 0 ] && exit 1 || exit 0