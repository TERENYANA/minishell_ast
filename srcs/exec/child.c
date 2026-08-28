/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   child.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:14:09 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:14:09 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	exec_cmd_in_child(t_node *root, t_node *cur, t_exec_info *info)
{
	int	status;

	expand_cmd_args(cur, *(info->env), info->last_status);
	if (apply_redirections(cur, *(info->env), info->last_status) != 0)
		cleanup_and_exit(root, info->env, 1);
	close_heredoc_fds(root);
	if (!cur->cmd || !cur->cmd[0])
		cleanup_and_exit(root, info->env, 0);
	if (is_builtin(cur->cmd[0]))
	{
		if (ft_strcmp(cur->cmd[0], "exit") == 0)
			status = ft_exit(root, cur, info->env, 0);
		else
			status = dispatch_builtin(cur, info->env, info->last_status);
		cleanup_and_exit(root, info->env, status);
	}
	exec_external_cmd(root, cur, info->env);
}

static void	exec_sub_in_child(t_node *root, t_node *cur, t_exec_info *info)
{
	if (apply_redirections(cur, *(info->env), info->last_status) != 0)
		cleanup_and_exit(root, info->env, 1);
	close_heredoc_fds(root);
	exec_node_in_child(root, cur->left, info);
}

void	exec_node_in_child(t_node *root, t_node *cur, t_exec_info *info)
{
	setup_child_signals();
	if (cur->type == N_CMD)
		exec_cmd_in_child(root, cur, info);
	else if (cur->type == N_SUB)
		exec_sub_in_child(root, cur, info);
	else if (cur->type == N_PIPE)
		exec_pipe_in_child(root, cur, info);
	else
		exec_andor_in_child(root, cur, info);
}

int	fork_and_run(t_node *root, t_var **env, int last_status)
{
	pid_t		pid;
	int			wstatus;
	int			status;
	t_exec_info	info;

	ignore_signals();
	pid = fork();
	if (pid == -1)
	{
		setup_signal_handlers();
		perror("minishell: fork");
		return (1);
	}
	if (pid == 0)
	{
		info.env = env;
		info.last_status = last_status;
		exec_node_in_child(root, root, &info);
	}
	waitpid(pid, &wstatus, 0);
	setup_signal_handlers();
	status = handle_child_status(wstatus);
	if (WIFSIGNALED(wstatus) && WTERMSIG(wstatus) == SIGINT)
		write(STDOUT_FILENO, "\n", 1);
	return (status);
}
