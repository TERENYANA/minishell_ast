/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 14:29:58 by masolet-          #+#    #+#             */
/*   Updated: 2025/12/04 15:45:59 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 42
#endif

static char	*extract_line(char **stash)
{
	char	*line;
	char	*rest;
	int		i;

	if (!*stash || !**stash)
	{
		if (*stash)
		{
			free(*stash);
			*stash = NULL;
		}
		return (NULL);
	}
	i = 0;
	while ((*stash)[i] && (*stash)[i] != '\n')
		i++;
	if ((*stash)[i] == '\n')
	{
		line = ft_substr(*stash, 0, i + 1);
		rest = ft_strdup(*stash + i + 1);
		free(*stash);
		*stash = rest;
		if (*stash && !**stash)
		{
			free(*stash);
			*stash = NULL;
		}
	}
	else
	{
		line = ft_strdup(*stash);
		free(*stash);
		*stash = NULL;
	}
	return (line);
}

static char	*read_buf(int fd, char *stash)
{
	char	buf[BUFFER_SIZE + 1];
	char	*tmp;
	ssize_t	r;

	r = 1;
	while (r > 0 && (!stash || !ft_strchr(stash, '\n')))
	{
		r = read(fd, buf, BUFFER_SIZE);
		if (r < 0)
		{
			if (stash)
				free(stash);
			return (NULL);
		}
		if (r == 0)
			break ;
		buf[r] = '\0';
		if (!stash)
			stash = ft_strdup(buf);
		else
		{
			tmp = ft_strjoin(stash, buf);
			free(stash);
			stash = tmp;
		}
	}
	return (stash);
}

char	*get_next_line(int fd)
{
	static char	*stash;

	if (fd < 0 || BUFFER_SIZE <= 0)
	{
		if (stash)
		{
			free(stash);
			stash = NULL;
		}
		return (NULL);
	}
	stash = read_buf(fd, stash);
	return (extract_line(&stash));
}