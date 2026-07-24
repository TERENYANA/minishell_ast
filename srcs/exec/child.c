#include "../minishell.h"

static int	is_directory(const char *path)
{
	struct stat	st;

	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return (1);
	return (0);
}

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

void	exec_pipe_in_child(t_node *root, t_node *cur, t_var **env)
{
	int		pf[2];
	pid_t	lpid;
	pid_t	rpid;
	int		wstatus;

	if (pipe(pf) == -1)
		cleanup_and_exit(root, env, 1);
	lpid = fork();
	if (lpid == 0)
		run_left(root, cur, env, pf);
	rpid = fork();
	if (rpid == 0)
		run_right(root, cur, env, pf);
	close(pf[0]);
	close(pf[1]);
	waitpid(lpid, NULL, 0);
	waitpid(rpid, &wstatus, 0);
	cleanup_and_exit(root, env, handle_child_status(wstatus));
}

void	exec_andor_in_child(t_node *root, t_node *cur, t_var **env)
{
	pid_t	pid;
	int		wstatus;
	int		status;

	if (apply_redirections(cur) != 0)
		cleanup_and_exit(root, env, 1);
	pid = fork();
	if (pid == 0)
		exec_node_in_child(root, cur->left, env);
	waitpid(pid, &wstatus, 0);
	status = handle_child_status(wstatus);
	if ((cur->type == N_AND && status == 0)
		|| (cur->type == N_OR && status != 0))
	{
		pid = fork();
		if (pid == 0)
			exec_node_in_child(root, cur->right, env);
		waitpid(pid, &wstatus, 0);
		status = handle_child_status(wstatus);
	}
	cleanup_and_exit(root, env, status);
}

void	exec_node_in_child(t_node *root, t_node *cur, t_var **env)
{
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

	pid = fork();
	if (pid == -1)
	{
		perror("minishell: fork");
		return (1);
	}
	if (pid == 0)
		exec_node_in_child(root, root, env);
	waitpid(pid, &wstatus, 0);
	return (handle_child_status(wstatus));
}