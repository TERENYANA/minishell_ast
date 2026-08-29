/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   child_exec.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:14:23 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:14:23 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
	if (!is_builtin(n->cmd[0]) && !is_env_builtin(n->cmd))
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
