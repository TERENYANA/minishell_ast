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

#include "../minishell.h"

static void	run_left(t_node *root, t_node *cur, t_var **env, int pf[2])
{
	close(pf[0]);
	dup2(pf[1], STDOUT_FILENO);
	close(pf[1]);
	exec_node_in_child(root, cur->left, env);
}

static void	run_right(t_node *root, t_node *cur, t_var **env, int pf[2])
{
	close(pf[1]);
	dup2(pf[0], STDIN_FILENO);
	close(pf[0]);
	exec_node_in_child(root, cur->right, env);
}

static pid_t	fork_left(t_node *root, t_node *cur, t_var **env, int pf[2])
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		close(pf[0]);
		close(pf[1]);
		cleanup_and_exit(root, env, 1);
	}
	if (pid == 0)
		run_left(root, cur, env, pf);
	return (pid);
}

static pid_t	fork_right(t_node *root, t_node *cur, t_var **env, int pf[2])
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		close(pf[0]);
		close(pf[1]);
		cleanup_and_exit(root, env, 1);
	}
	if (pid == 0)
		run_right(root, cur, env, pf);
	return (pid);
}

void	exec_pipe_in_child(t_node *root, t_node *cur, t_var **env)
{
	int		pf[2];
	pid_t	lpid;
	pid_t	rpid;
	int		wstatus;

	if (pipe(pf) == -1)
		cleanup_and_exit(root, env, 1);
	lpid = fork_left(root, cur, env, pf);
	rpid = fork_right(root, cur, env, pf);
	close(pf[0]);
	close(pf[1]);
	waitpid(lpid, &wstatus, 0);
	waitpid(rpid, &wstatus, 0);
	cleanup_and_exit(root, env, handle_child_status(wstatus));
}
