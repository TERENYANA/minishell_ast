/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   child.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:41:03 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:41:06 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static void	exec_cmd_in_child(t_node *root, t_node *cur, t_var **env)
{
	int	status;

	if (apply_redirections(cur) != 0)
		cleanup_and_exit(root, env, 1);
	if (!cur->cmd || !cur->cmd[0])
		cleanup_and_exit(root, env, 0);
	if (is_builtin(cur->cmd[0]))
	{
		if (ft_strcmp(cur->cmd[0], "exit") == 0)
			status = ft_exit(root, cur, env, 0);
		else
			status = dispatch_builtin(cur, env, 0);
		cleanup_and_exit(root, env, status);
	}
	exec_external_cmd(root, cur, env);
}

static void	exec_sub_in_child(t_node *root, t_node *cur, t_var **env)
{
	if (apply_redirections(cur) != 0)
		cleanup_and_exit(root, env, 1);
	exec_node_in_child(root, cur->left, env);
}

void	exec_node_in_child(t_node *root, t_node *cur, t_var **env)
{
	setup_child_signals();
	if (cur->type == N_CMD)
		exec_cmd_in_child(root, cur, env);
	else if (cur->type == N_SUB)
		exec_sub_in_child(root, cur, env);
	else if (cur->type == N_PIPE)
		exec_pipe_in_child(root, cur, env);
	else
		exec_andor_in_child(root, cur, env);
}

int	fork_and_run(t_node *root, t_var **env)
{
	pid_t	pid;
	int		wstatus;
	int		status;

	ignore_signals();
	pid = fork();
	if (pid == -1)
	{
		setup_signal_handlers();
		perror("minishell: fork");
		return (1);
	}
	if (pid == 0)
		exec_node_in_child(root, root, env);
	waitpid(pid, &wstatus, 0);
	setup_signal_handlers();
	status = handle_child_status(wstatus);
	if (WIFSIGNALED(wstatus) && WTERMSIG(wstatus) == SIGINT)
		write(STDOUT_FILENO, "\n", 1);
	return (status);
}
