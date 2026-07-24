#include "../minishell.h"

static int	write_pipe(int fd, char *line, t_redirect *r, t_var *env, int status)
{
	char	*expanded;

	if (r->expand_heredoc)
	{
		expanded = expand_heredoc_line(line, env, status);
		if (!expanded)
		{
			free(line);
			return (-1);
		}
		write(fd, expanded, ft_strlen(expanded));
		write(fd, "\n", 1);
		free(expanded);
		free(line);
	}
	else
	{
		write(fd, line, ft_strlen(line));
		write(fd, "\n", 1);
		free(line);
	}
	return (0);
}

static int	hd_line(int p[2], char *line, t_redirect *r, t_var *env, int status)
{
	if (!line)
		return (0);
	if (ft_strcmp(line, r->target) == 0)
	{
		free(line);
		return (0);
	}
	if (write_pipe(p[1], line, r, env, status) == -1)
	{
		close(p[1]);
		return (-1);
	}
	return (1);
}

static void	heredoc_child(int p[2], t_redirect *redir, t_var *env, int status)
{
	char	*line;
	int		ret;

	close(p[0]);
	setup_heredoc_signals();
	while (1)
	{
		if (isatty(STDIN_FILENO))
			line = readline("> ");
		else
			line = get_next_line(STDIN_FILENO);
		ret = hd_line(p, line, redir, env, status);
		if (ret <= 0)
		{
			close(p[1]);
			if (ret == -1)
				exit(1);
			exit(0);
		}
	}
}

static int	process_heredoc(t_redirect *redir, t_var *env, int status_val)
{
	int		p[2];
	pid_t	pid;
	int		status;

	if (pipe(p) == -1)
		return (-1);
	ignore_signals();
	pid = fork();
	if (pid == -1)
		return (close(p[0]), close(p[1]), -1);
	if (pid == 0)
		heredoc_child(p, redir, env, status_val);
	close(p[1]);
	waitpid(pid, &status, 0);
	setup_signal_handlers();
	if ((WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		|| (WIFEXITED(status) && WEXITSTATUS(status) == 130))
	{
		g_sig = SIGINT;
		close(p[0]);
		return (-1);
	}
	redir->heredoc_fd = p[0];
	return (0);
}

int	process_all_heredocs(t_node *node, t_var *env, int status)
{
	t_redirect	*r;

	if (!node)
		return (0);
	if (node->type == N_CMD)
	{
		r = node->redirect;
		while (r)
		{
			if (r->type == HEREDOC && process_heredoc(r, env, status) == -1)
				return (-1);
			r = r->next;
		}
	}
	else
	{
		if (process_all_heredocs(node->left, env, status) == -1)
			return (-1);
		if (process_all_heredocs(node->right, env, status) == -1)
			return (-1);
	}
	return (0);
}