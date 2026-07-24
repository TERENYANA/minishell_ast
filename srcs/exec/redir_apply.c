#include "../minishell.h"

static int	open_redir(t_redirect *r)
{
	if (r->type == HEREDOC)
		return (r->heredoc_fd);
	if (r->type == REDIR_IN)
		return (open(r->target, O_RDONLY));
	if (r->type == REDIR_APPEND)
		return (open(r->target, O_WRONLY | O_CREAT | O_APPEND, 0644));
	return (open(r->target, O_WRONLY | O_CREAT | O_TRUNC, 0644));
}

static int	perror_redir(const char *target)
{
	ft_putstr_fd("minishell: ", 2);
	perror((char *)target);
	return (1);
}

int	apply_redirections(t_node *cmd_node)
{
	t_redirect	*r;
	int			fd;

	r = cmd_node->redirect;
	while (r)
	{
		fd = open_redir(r);
		if (fd < 0)
			return (perror_redir(r->target));
		if (r->type == REDIR_IN || r->type == HEREDOC)
			dup2(fd, STDIN_FILENO);
		else
			dup2(fd, STDOUT_FILENO);
		if (r->type != HEREDOC)
			close(fd);
		r = r->next;
	}
	return (0);
}