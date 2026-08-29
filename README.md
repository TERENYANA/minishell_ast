*This project has been created as part of the 42 curriculum by <yyuskiv>, <masolet->.*

# Minishell

## Description
Minishell is a project from 42 that challenges you to build a simple but functional UNIX shell from scratch. The main goal is to understand how a shell operates, including process creation, execution, synchronization, and the parsing of complex command lines.

In this project, we implemented a custom shell that supports basic features found in bash, such as executing commands, handling arguments, managing environment variables, pipes (`|`), redirections (`<`, `>`, `<<`, `>>`), and several built-in commands (`echo`, `cd`, `pwd`, `export`, `unset`, `env`, `exit`).

## Instructions

### Prerequisites
- A UNIX-like operating system (Linux)
- GCC or Clang compiler
- `make` utility
- `readline` library installed on your system

### Compilation

```bash
git clone <url> minishell
cd minishell
make
```

This will create an executable file named `minishell`.
Additional Makefile rules available:
- `make clean`: Removes object files.
- `make fclean`: Removes object files and the executable.
- `make re`: Recompiles the entire project.

### Execution
Run the compiled executable to start minishell:

```bash
./minishell
```

You will be greeted with a custom prompt, and you can start typing commands just like in bash.

## Resources

### Classic References
- **GNU Bash Reference Manual**: Essential for understanding shell behavior and POSIX standards.
- **Linux man pages**: Essential reading for functions like `fork()`, `execve()`, `pipe()`, `dup2()`, `waitpid()`, etc.
- **Understanding AST (Abstract Syntax Trees)**: Helpful for building the parser and executing commands structurally.

### Use of AI
- **Debugging**: Assisting in tracking down memory leaks (using valgrind output) and resolving edge cases in string parsing.
- **Refactoring**: Providing suggestions on structuring the Abstract Syntax Tree (AST) logic and organizing the codebase to adhere to norminette.
- **README**: Assisting to write and translate this file.

