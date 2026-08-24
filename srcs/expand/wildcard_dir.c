/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wildcard_dir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:49:05 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:49:07 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"
#include <dirent.h>

static int	visible_match(const char *pat, const char *name)
{
	if (name[0] == '.' && pat[0] != '.')
		return (0);
	if ((ft_strcmp(name, ".") == 0 || ft_strcmp(name, "..") == 0)
		&& ft_strcmp(pat, name) != 0)
		return (0);
	return (wc_match(pat, name));
}

char	**collect_matches(const char *pattern)
{
	DIR				*dir;
	struct dirent	*e;
	char			**tab;
	char			*dir_path;
	char			*file_pat;
	char			*last_slash;
	char			*full_match;

	tab = ft_calloc(1, sizeof(char *));
	if (!tab)
		return (NULL);

	last_slash = ft_strrchr(pattern, '/');
	if (last_slash)
	{
		dir_path = ft_substr(pattern, 0, last_slash - pattern + 1);
		file_pat = last_slash + 1;
	}
	else
	{
		dir_path = ft_strdup(".");
		file_pat = (char *)pattern;
	}

	dir = opendir(dir_path);
	if (!dir)
		return (free(dir_path), tab);

	e = readdir(dir);
	while (e)
	{
		if (visible_match(file_pat, e->d_name))
		{
			if (last_slash)
				full_match = ft_strjoin(dir_path, e->d_name);
			else
				full_match = ft_strdup(e->d_name);
			if (!full_match || !tab_push(&tab, full_match))
			{
				free(full_match);
				free(dir_path);
				closedir(dir);
				ft_free_tab(tab);
				return (NULL);
			}
			free(full_match);
		}
		e = readdir(dir);
	}
	free(dir_path);
	closedir(dir);
	return (tab);
}

void	sort_tab(char **tab)
{
	int		i;
	char	*tmp;

	i = 0;
	while (tab[i] && tab[i + 1])
	{
		if (ft_strcmp(tab[i], tab[i + 1]) > 0)
		{
			tmp = tab[i];
			tab[i] = tab[i + 1];
			tab[i + 1] = tmp;
			i = 0;
		}
		else
			i++;
	}
}
