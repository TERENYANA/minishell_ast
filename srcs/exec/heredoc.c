#include "../minishell.h"

static int	err_msg_heredoc(char *eof)
{
	if (g_sig == SIGINT)
		return (-1);
	ft_putstr_fd("minishell: warning: here-document ", 2);
	ft_putstr_fd("delimited by end-of-file (wanted `", 2);
	if (eof)
		ft_putstr_fd(eof, 2);
	ft_putstr_fd("')\n", 2);
	return (0);
}

static int	write_pipe(int fd, char *line, t_redirect *r, t_var *env)
{
	char	*expanded;

	if (r->expand_heredoc)
	{
		expanded = expand_heredoc_line(line, env);
		if (!expanded)
			return (-1);
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

static int	hd_line(int p[2], char *line, t_redirect *r, t_var *env)
{
	if (!line)
	{
		err_msg_heredoc(r->target);
		return (0);
	}
	if (ft_strcmp(line, r->target) == 0)
		return (free(line), 0);
	if (write_pipe(p[1], line, r, env) == -1)
		return (close(p[0]), close(p[1]), -1);
	return (1);
}

static int	process_heredoc(t_redirect *redir, t_var *env)
{
	int		p[2];
	char	*line;
	int		ret;

	if (pipe(p) == -1)
		return (-1);
	while (1)
	{
		if (g_sig)
			return (close(p[0]), close(p[1]), -1);
		line = readline("> ");
		ret = hd_line(p, line, redir, env);
		if (ret == -1)
			return (-1);
		if (ret == 0)
			break ;
	}
	close(p[1]);
	redir->heredoc_fd = p[0];
	return (0);
}

int	process_all_heredocs(t_node *node, t_var *env, int status)
{
	t_redirect	*r;

	(void)status;
	if (!node)
		return (0);
	if (node->type == N_CMD)
	{
		r = node->redirect;
		while (r)
		{
			if (r->type == HEREDOC && process_heredoc(r, env) == -1)
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
