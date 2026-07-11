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

static char	*read_to_stash(int fd, char *line, char *stash, ssize_t *n)
{
	char	*tmp;

	*n = 1;
	stash[0] = '\0';
	while (!ft_strchr(stash, '\n'))
	{
		if (stash[0])
		{
			tmp = line;
			line = ft_strjoin(line, stash);
			free(tmp);
			if (!line)
				return (NULL);
		}
		*n = read(fd, stash, BUFFER_SIZE);
		if (*n < 0)
			return (free(line), NULL);
		stash[*n] = '\0';
		if (*n == 0)
			break ;
	}
	return (line);
}

static char	*extract_from_stash(char *line, char *stash)
{
	size_t	i;
	size_t	j;
	char	*line_tmp;
	char	*tmp;

	i = 0;
	j = 0;
	tmp = ft_calloc(BUFFER_SIZE + 1, sizeof(char));
	if (!tmp)
		return (free(line), NULL);
	while (stash[i])
	{
		tmp[j++] = stash[i];
		if (stash[i++] == '\n')
			break ;
	}
	line_tmp = line;
	line = ft_strjoin(line, tmp);
	(free(line_tmp), free(tmp), j = 0);
	if (!line)
		return (NULL);
	while (stash[i])
		stash[j++] = stash[i++];
	stash[j] = '\0';
	return (line);
}

static char	*fill_line(int fd, char *line, char *stash)
{
	ssize_t	n;

	line = read_to_stash(fd, line, stash, &n);
	if (!line)
		return (NULL);
	if (n >= 0)
		line = extract_from_stash(line, stash);
	if (!line || line[0] == '\0')
	{
		free(line);
		return (NULL);
	}
	return (line);
}

static void	clean_stash(char *stash)
{
	size_t	i;
	size_t	j;

	i = 0;
	j = 0;
	while (stash[i] && stash[i] != '\n')
		i++;
	if (stash[i] == '\n')
		i++;
	while (stash[i])
		stash[j++] = stash[i++];
	stash[j] = '\0';
}

char	*get_next_line(int fd)
{
	static char	stash[BUFFER_SIZE + 1];
	char		*line;
	size_t		i;

	line = ft_calloc(ft_strlen(stash) + 1, sizeof(char));
	if (!line)
		return (NULL);
	if (stash[0])
	{
		i = 0;
		while (stash[i] && stash[i] != '\n')
		{
			line[i] = stash[i];
			i++;
		}
		if (stash[i] == '\n')
			line[i] = stash[i];
	}
	if (ft_strchr(line, '\n'))
		return (clean_stash(stash), line);
	line = fill_line(fd, line, stash);
	return (line);
}
