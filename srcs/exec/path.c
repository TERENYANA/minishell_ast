/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   path.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:43:35 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/29 15:23:10 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Checks whether the given filesystem path is a directory.
 */
static int	is_directory(const char *path)
{
	struct stat	st;

	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return (1);
	return (0);
}

/*
 * Joins a directory path and a command name into a full executable path.
 */
static char	*join_path(const char *dir, const char *cmd)
{
	char	*tmp;
	char	*res;

	tmp = ft_strjoin((char *)dir, "/");
	if (!tmp)
		return (NULL);
	res = ft_strjoin(tmp, (char *)cmd);
	free(tmp);
	return (res);
}

/*
 * Scans every PATH directory entry and returns the first executable file
 * matching the command name while skipping directories.
 */
static char	*search_in_paths(char **dirs, const char *cmd)
{
	char	*full;
	int		i;

	i = 0;
	while (dirs && dirs[i])
	{
		full = join_path(dirs[i], cmd);
		if (full && access(full, X_OK) == 0 && !is_directory(full))
			return (full);
		free(full);
		i++;
	}
	return (NULL);
}

/*
 * Resolves the full path of a command by checking direct paths first,
 * then searching through PATH directories to find an executable match.
 */
char	*find_cmd_path(char *cmd, t_var *env)
{
	char	**dirs;
	char	*path;
	char	*found;

	if (!cmd || !cmd[0])
		return (NULL);
	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, F_OK) != 0)
			return (NULL);
		return (ft_strdup(cmd));
	}
	path = get_env_value("PATH", env);
	if (!path)
		return (NULL);
	dirs = ft_split(path, ':');
	if (!dirs)
		return (NULL);
	found = search_in_paths(dirs, cmd);
	ft_free_tab(dirs);
	return (found);
}
