#include "../minishell.h"

/*
** FUNCTION: is_directory
** ----------------------
** Helper function checking whether a given path points to a directory.
**
** Param: path - Path string to inspect (e.g., "/usr/bin" or "./my_folder").
**
** Return: 1 (true) if path exists and is a directory, 0 (false) otherwise.
**
** HOW IT WORKS:
** Uses system call `stat()` to retrieve file metadata, then `S_ISDIR()` 
** macro to check if file mode flags indicate a directory.
**
** EXAMPLE TRACE:
** Input: path = "/usr/bin"
** 1. stat("/usr/bin", &st) succeeds (returns 0).
** 2. S_ISDIR(st.st_mode) evaluates to TRUE.
** 3. Returns 1.
*/
static int	is_directory(const char *path)
{
	struct stat	st;

	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return (1);
	return (0);
}

/*
** FUNCTION: exec_external_cmd
** ---------------------------
** Resolves, validates, and executes non-builtin programs (e.g., `ls`, `grep`, `./a.out`).
**
** Param: root - Pointer to full AST root (needed to free memory on exit).
** Param: cur  - Pointer to current command AST node containing `cmd` array.
** Param: env  - Pointer to environment structure pointer.
**
** Return: Does not return on success (`execve` replaces process image).
**         Exits child process with error status (126 or 127) on failure.
**
** POSIX EXIT CODES:
** - 127: Command not found.
** - 126: File is found/given but cannot be executed (e.g., permission denied, or path is a directory).
**
** EXAMPLE TRACE (`ls -l`):
** 1. `path = find_cmd_path("ls", *env)` -> returns "/bin/ls".
** 2. `is_directory("/bin/ls")` returns 0 (it's a binary file, not a directory).
** 3. `envp = convert_env_list(*env)` converts linked list to NULL-terminated `char **`.
** 4. `execve("/bin/ls", ["ls", "-l", NULL], envp)` replaces current process image with `ls`.
*/
void	exec_external_cmd(t_node *root, t_node *cur, t_var **env)
{
	char	*path;
	char	**envp;

	path = find_cmd_path(cur->cmd[0], *env);
	if (!path)
	{
		err_msg(cur->cmd[0], "command not found", 0);
		cleanup_and_exit(root, env, 127);
	}
	if (is_directory(path))
	{
		err_msg(cur->cmd[0], "Is a directory", 0);
		free(path);
		cleanup_and_exit(root, env, 126);
	}
	envp = convert_env_list(*env);
	execve(path, cur->cmd, envp);
	perror(cur->cmd[0]);
	free(path);
	ft_free_tab(envp);
	cleanup_and_exit(root, env, 126);
}

/*
** FUNCTION: exec_cmd_in_child
** ---------------------------
** Evaluates a simple command node inside a child process.
** Handles built-ins executing inside pipelines/children and delegates external commands.
**
** Param: root - Pointer to full AST root.
** Param: cur  - Pointer to current command node.
** Param: env  - Pointer to environment structure pointer.
**
** Return: None (Always terminates child process via `cleanup_and_exit`).
**
** EXAMPLE TRACE (`echo hello` in child):
** 1. Checks `cur->cmd` exists.
** 2. `is_builtin("echo")` returns 1 (true).
** 3. Calls `dispatch_builtin(cur, env, 0)`.
** 4. Calls `cleanup_and_exit(root, env, status)` to terminate child cleanly.
*/
static void	exec_cmd_in_child(t_node *root, t_node *cur, t_var **env)
{
	int	status;

	if (!cur->cmd || !cur->cmd[0])
		cleanup_and_exit(root, env, 0);
	if (is_builtin(cur->cmd[0]))
	{
		status = dispatch_builtin(cur, env, 0);
		cleanup_and_exit(root, env, status);
	}
	exec_external_cmd(root, cur, env);
}

/*
** FUNCTION: exec_node_in_child
** ----------------------------
** Entry point for executing any AST node type within a child process context.
**
** Param: root - Pointer to full AST root node.
** Param: cur  - Pointer to current sub-tree node to execute.
** Param: env  - Pointer to environment structure pointer.
**
** Return: None.
**
** EXAMPLE TRACE:
** Input: node of type `N_CMD` (`ls -la`).
** 1. Checks `cur->type == N_CMD`.
** 2. Passes control to `exec_cmd_in_child`.
*/
void	exec_node_in_child(t_node *root, t_node *cur, t_var **env)
{
	if (cur->type == N_CMD)
		exec_cmd_in_child(root, cur, env);
}