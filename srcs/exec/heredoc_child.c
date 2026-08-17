#include "../minishell.h"

static char	*hd_read_line(void)
{
	char	*line;
	size_t	len;

	if (isatty(STDIN_FILENO))
		return (readline("> "));
	line = get_next_line(STDIN_FILENO);
	if (!line)
		return (NULL);
	len = ft_strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len - 1] = '\0';
	return (line);
}

static int	write_pipe(int fd, char *line, t_hd *hd)
{
	char	*expanded;

	if (hd->redir->expand_heredoc)
	{
		expanded = expand_heredoc_line(line, hd->env, hd->status);
		if (!expanded)
			return (free(line), -1);
		write(fd, expanded, ft_strlen(expanded));
		write(fd, "\n", 1);
		free(expanded);
		free(line);
		return (0);
	}
	write(fd, line, ft_strlen(line));
	write(fd, "\n", 1);
	free(line);
	return (0);
}

static int	hd_line(int p[2], char *line, t_hd *hd)
{
	if (!line)
		return (0);
	if (ft_strcmp(line, hd->redir->target) == 0)
		return (free(line), 0);
	if (write_pipe(p[1], line, hd) == -1)
		return (close(p[1]), -1);
	return (1);
}

void	heredoc_child(int p[2], t_hd *hd)
{
	char	*line;
	int		ret;

	close(p[0]);
	setup_heredoc_signals();
	while (1)
	{
		line = hd_read_line();
		ret = hd_line(p, line, hd);
		if (ret <= 0)
		{
			close(p[1]);
			if (ret == -1)
				exit(1);
			exit(0);
		}
	}
}