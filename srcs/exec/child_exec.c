/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   child_exec.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:42:38 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/20 11:41:37 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

 
/* Returns 1 if path is a directory, 0 otherwise. */
static int	is_directory(const char *path)
{
	struct stat	st;
 
	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return (1);
	return (0);
}
 
/* Resolves the full path of a command or exits with 127/126.
** - No path found    → "command not found"  → exit 127
** - Path is a dir   → "is a directory"      → exit 126 */
static char	*cmd_path_or_exit(t_node *root, t_node *cur, t_var **env)
{
	char	*path;
 
	path = find_cmd_path(cur->cmd[0], *env);
	if (!path)
	{
		if (ft_strchr(cur->cmd[0], '/'))
			err_msg(cur->cmd[0], "No such file or directory", 0);
		else
			err_msg(cur->cmd[0], "command not found", 0);
		cleanup_and_exit(root, env, 127);
	}
	if (is_directory(path))
	{
		err_msg(cur->cmd[0], "is a directory", 0);
		free(path);
		cleanup_and_exit(root, env, 126);
	}
	return (path);
}
 
/* Runs an external command via execve.
** convert_env_list builds the char** envp from our t_var list.
** execve never returns on success — code after it only runs on error. */
void	exec_external_cmd(t_node *root, t_node *cur, t_var **env)
{
	char	*path;
	char	**envp;
	int		exit_code;
 
	path = cmd_path_or_exit(root, cur, env);
	envp = convert_env_list(*env);
	if (!envp)
	{
		free(path);
		cleanup_and_exit(root, env, 1);
	}
	execve(path, cur->cmd, envp);
	perror(cur->cmd[0]);
	exit_code = 126;
	if (errno == ENOENT)
		exit_code = 127;
	free(path);
	ft_free_tab(envp);
	cleanup_and_exit(root, env, exit_code);
}
 
/* Forks a child to run node n, waits for it and returns its exit status.
** Used for external commands and complex nodes (pipe, sub) inside && / ||
** where we need an exit status without replacing the current process. */
static int	run_subtree(t_node *root, t_node *n, t_exec_info *info)
{
	pid_t	pid;
	int		wstatus;
 
	pid = fork();
	if (pid == -1)
		return (1);
	if (pid == 0)
		exec_node_in_child(root, n, info);
	waitpid(pid, &wstatus, 0);
	return (handle_child_status(wstatus));
}
 
/* Runs a single N_CMD node directly in the current process (no fork).
** This is essential for builtins like cd inside (cd / && pwd):
** if cd ran in a fork, the chdir would be lost when that fork exits.
** External commands still need a fork because execve replaces the process. */
static int	run_cmd_direct(t_node *root, t_node *n, t_exec_info *info)
{
	int	status;
 
	expand_cmd_args(n, *(info->env), info->last_status);
	if (apply_redirections(n, *(info->env), info->last_status) != 0)
		return (1);
	if (!n->cmd || !n->cmd[0])
		return (0);
	if (!is_builtin(n->cmd[0]))
		return (run_subtree(root, n, info));
	if (ft_strcmp(n->cmd[0], "exit") == 0)
		status = ft_exit(root, n, info->env, info->last_status);
	else
		status = dispatch_builtin(n, info->env, info->last_status);
	return (status);
}
 
/* Picks execution strategy for one side of && / ||.
** N_CMD → run_cmd_direct (no fork, so builtins affect current process).
** Anything else (pipe, sub, nested &&/||) → run_subtree (needs fork). */
static int	run_side(t_node *root, t_node *n, t_exec_info *info)
{
	if (n->type == N_CMD)
		return (run_cmd_direct(root, n, info));
	return (run_subtree(root, n, info));
}
 
/* Handles && and || inside a child process (subshell or andor fork).
** Runs left side, then conditionally runs right side based on exit status.
** Uses run_side so builtins (cd, export...) affect the subshell state. */
void	exec_andor_in_child(t_node *root, t_node *cur, t_exec_info *info)
{
	int	status;
 
	status = run_side(root, cur->left, info);
	if ((cur->type == N_AND && status == 0)
		|| (cur->type == N_OR && status != 0))
	{
		info->last_status = status;
		status = run_side(root, cur->right, info);
	}
	cleanup_and_exit(root, info->env, status);
}
