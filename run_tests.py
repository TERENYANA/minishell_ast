import subprocess

parsing = [
    "echo ",
    "echo hello world !",
    "echo 42test ' 21 '",
    "echo 42   42    '  '42",
    "echo 'echo $v >> file.txt'",
    "echo ' \"\" ' '42'",
    "echo '$?'",
    "echo '${$?}'",
    "echo 42' '42",
    "echo '$USER'",
    "echo '<< | | >>'42",
    "echo ''",
    "echo ''''42''",
    "echo $",
    "echo '$'",
    "echo \"42\"'$'\"42\"",
    "echo a'b'c'd'e'f'g'h'i'j'k'l'm'n'o'p'q'r's't'",
    "echo \" \"'$USER\"'\"42 \" ''\"  | << -1",
    "echo \"<< EOF\"",
    "echo $HOME",
    "echo $HOME$HOME",
    "echo $USER'$USER'",
    "echo $USER \"$HOME\"",
    "echo $USER42",
    "echo '$USER \"$HOME\"'",
    "echo $USER '>> file.txt' \"|\"",
    "echo '42 $USER' \">\" file.txt",
    "echo \"$USER 42\" '\"$USER\"'",
    "echo $USER42",
    "echo ''\"\"'\"'\"'\""
]

def run_test_case(cmd, binary="./minishell"):
    # Run in minishell
    p = subprocess.Popen(
        [binary], 
        stdin=subprocess.PIPE, 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, 
        text=True
    )
    mini_out, mini_err = p.communicate(input=cmd + "\nexit\n")
    mini_exit = p.returncode

    # Run in reference bash
    b = subprocess.run(
        ["bash", "--norc", "--noprofile"], 
        input=cmd + "\nexit\n", 
        capture_output=True, 
        text=True
    )
    bash_out = b.stdout

    return mini_out, mini_err, bash_out

def main():
    binary = "./minishell"
    print(f"Running {len(parsing)} test cases against {binary}...\n")
    
    for i, cmd in enumerate(parsing):
        mini_out, mini_err, bash_out = run_test_case(cmd, binary)
        
        # Clean up output formatting for comparison
        mini_cleaned = "\n".join([line for line in mini_out.splitlines() if not line.startswith("minishell$")])
        
        print(f"[{i + 1:02d}] Test: {cmd}")
        print(f"      Minishell Output: {repr(mini_cleaned.strip())}")
        if mini_err:
            print(f"      Minishell Error:  {repr(mini_err.strip())}")
        print(f"      Bash Output:      {repr(bash_out.strip())}")
        print("-" * 60)

if __name__ == "__main__":
    main()