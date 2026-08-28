/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   path.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:43:35 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/20 11:23:05 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_directory(const char *path)
{
	struct stat	st;

	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return (1);
	return (0);
}

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
