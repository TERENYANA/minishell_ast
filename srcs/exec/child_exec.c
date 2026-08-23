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

#include "../minishell.h"

static int	is_directory(const char *path)
{
	struct stat	st;

	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return (1);
	return (0);
}

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

static int	run_subtree(t_node *root, t_node *n, t_var **env, int last_status)
{
	pid_t	pid;
	int		wstatus;

	pid = fork();
	if (pid == -1)
		return (1);
	if (pid == 0)
		exec_node_in_child(root, n, env, last_status);
	waitpid(pid, &wstatus, 0);
	return (handle_child_status(wstatus));
}

void	exec_andor_in_child(t_node *root, t_node *cur, t_var **env, int last_status)
{
	int	status;

	status = run_subtree(root, cur->left, env, last_status);
	if ((cur->type == N_AND && status == 0)
		|| (cur->type == N_OR && status != 0))
		status = run_subtree(root, cur->right, env, status); // Pass status as last_status for the right side!
	cleanup_and_exit(root, env, status);
}
