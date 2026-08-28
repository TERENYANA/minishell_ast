/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   child_pipe.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:42:49 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:42:51 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	run_left(t_node *root, t_node *cur, int pf[2], t_exec_info *info)
{
	close(pf[0]);
	dup2(pf[1], STDOUT_FILENO);
	close(pf[1]);
	exec_node_in_child(root, cur->left, info);
}

static void	run_right(t_node *root, t_node *cur, int pf[2], t_exec_info *info)
{
	close(pf[1]);
	dup2(pf[0], STDIN_FILENO);
	close(pf[0]);
	exec_node_in_child(root, cur->right, info);
}

static pid_t	fork_left(t_node *root, t_node *cur, int pf[2],
		t_exec_info *info)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		close(pf[0]);
		close(pf[1]);
		cleanup_and_exit(root, info->env, 1);
	}
	if (pid == 0)
		run_left(root, cur, pf, info);
	return (pid);
}

static pid_t	fork_right(t_node *root, t_node *cur, int pf[2],
		t_exec_info *info)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		close(pf[0]);
		close(pf[1]);
		cleanup_and_exit(root, info->env, 1);
	}
	if (pid == 0)
		run_right(root, cur, pf, info);
	return (pid);
}

void	exec_pipe_in_child(t_node *root, t_node *cur, t_exec_info *info)
{
	int		pf[2];
	pid_t	lpid;
	pid_t	rpid;
	int		st_right;

	if (pipe(pf) == -1)
		cleanup_and_exit(root, info->env, 1);
	lpid = fork_left(root, cur, pf, info);
	rpid = fork_right(root, cur, pf, info);
	close(pf[0]);
	close(pf[1]);
	waitpid(lpid, NULL, 0);
	waitpid(rpid, &st_right, 0);
	cleanup_and_exit(root, info->env, handle_child_status(st_right));
}
