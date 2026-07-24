#include "../minishell.h"

/*
** FUNCTION: run_builtin_in_parent
** --------------------------------
** Runs shell built-in commands (like `cd`, `exit`, `export`, `unset`) 
** directly inside the parent process instead of spawning a fork.
**
** WHY PARENT PROCESS?
** Built-ins modify the shell's state (e.g., changing working directory or environment).
** If run inside a child process, those changes would be lost when the child exits.
**
** Param: node        - Pointer to AST node containing command & args.
** Param: env         - Pointer to environment structure pointer.
** Param: last_status - Exit status of the previously executed command (for `$?`).
**
** Return: Exit status integer returned by the executed built-in command.
**
** EXAMPLE TRACE (`cd /tmp`):
** 1. Checks if `node->cmd` or `node->cmd[0]` ("cd") is NULL (returns 0 if empty).
** 2. Calls `dispatch_builtin(node, env, last_status)`.
** 3. `dispatch_builtin` executes `chdir("/tmp")` and updates `env`.
** 4. Returns 0 (success).
*/
static int	run_builtin_in_parent(t_node *node, t_var **env, int last_status)
{
	if (!node->cmd || !node->cmd[0])
		return (0);
	return (dispatch_builtin(node, env, last_status));
}

/*
** FUNCTION: is_parent_builtin_root
** ---------------------------------
** Predicate helper function checking if the current root node of the AST
** represents a single built-in command that MUST run in the parent shell.
**
** Param: root - AST root node to check.
**
** Return: 1 (true) if root is a simple command AND its binary is a built-in.
**         0 (false) if it is an external program (e.g., `ls`, `grep`) or complex pipeline.
**
** EXAMPLE TRACE:
** Input: `export VAR=42`
** - `root->type == N_CMD` (Simple command node)
** - `root->cmd[0] = "export"`
** - `is_builtin("export")` returns 1 -> Function returns 1 (true).
*/
static int	is_parent_builtin_root(t_node *root)
{
	return (root->type == N_CMD
		&& root->cmd && root->cmd[0]
		&& is_builtin(root->cmd[0]));
}

/*
** FUNCTION: run_tree
** -------------------
** Top-level execution manager for the AST (Abstract Syntax Tree).
** Decides whether to run in the parent process (for single built-ins) 
** or spawn a child process for external programs.
**
** Param: root        - AST root node.
** Param: env         - Pointer to environment structure pointer.
** Param: last_status - Exit code of previous command.
**
** Return: Exit code of the executed command (0-255).
**
** EXAMPLE TRACE 1 (Running `cd /usr` - Parent path):
** 1. `is_parent_builtin_root(root)` checks if "cd" is a built-in -> returns 1 (true).
** 2. Directly runs `run_builtin_in_parent(...)` and returns its result.
**
** EXAMPLE TRACE 2 (Running `ls -l` - Child fork path):
** 1. `is_parent_builtin_root(root)` returns 0.
** 2. `ignore_signals()` prevents parent shell from reacting to signals during execution.
** 3. `pid = fork()` creates child process.
**
**    [CHILD PROCESS (pid == 0)]:
**    - Resets SIGINT/SIGQUIT to standard default behavior (`SIG_DFL`).
**    - `exec_node_in_child()` searches PATH and executes `/bin/ls`.
**    - If `exec` fails, calls `cleanup_and_exit(root, env, 127)`.
**
**    [PARENT PROCESS (pid > 0)]:
**    - `waitpid(...)` waits for child process to complete.
**    - `handle_child_status(wstatus)` extracts normalized exit code.
**    - `setup_signal_handlers()` restores prompt interactive signal handlers.
**    - Returns status code.
*/
int	run_tree(t_node *root, t_var **env, int last_status)
{
	pid_t	pid;
	int		wstatus;
	int		status;

	if (is_parent_builtin_root(root))
		return (run_builtin_in_parent(root, env, last_status));
	ignore_signals();
	pid = fork();
	if (pid < 0)
		return (perror("fork"), setup_signal_handlers(), 1);
	if (pid == 0)
	{
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		exec_node_in_child(root, root, env);
		cleanup_and_exit(root, env, 127);
	}
	if (waitpid(pid, &wstatus, 0) == -1)
		status = 1;
	else
		status = handle_child_status(wstatus);
	setup_signal_handlers();
	return (status);
}