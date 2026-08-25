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

static char	**read_and_match(DIR *dir, char *d_p, char *f_p, char **tab)
{
	struct dirent	*e;
	char			*full;

	e = readdir(dir);
	while (e)
	{
		if (visible_match(f_p, e->d_name))
		{
			if (d_p)
				full = ft_strjoin(d_p, e->d_name);
			else
				full = ft_strdup(e->d_name);
			if (!full || !tab_push(&tab, full))
				return (free(full), ft_free_tab(tab), NULL);
			free(full);
		}
		e = readdir(dir);
	}
	return (tab);
}

static DIR	*init_match(const char *pat, char **d_p, char **f_p, char **ls)
{
	*ls = ft_strrchr(pat, '/');
	if (*ls)
	{
		*d_p = ft_substr(pat, 0, *ls - pat + 1);
		*f_p = *ls + 1;
	}
	else
	{
		*d_p = ft_strdup(".");
		*f_p = (char *)pat;
	}
	return (opendir(*d_p));
}

char	**collect_matches(const char *pattern)
{
	DIR		*dir;
	char	*dir_path;
	char	*file_pat;
	char	*last_slash;
	char	**tab;

	tab = ft_calloc(1, sizeof(char *));
	if (!tab)
		return (NULL);
	dir = init_match(pattern, &dir_path, &file_pat, &last_slash);
	if (!dir)
		return (free(dir_path), tab);
	if (!last_slash)
		tab = read_and_match(dir, NULL, file_pat, tab);
	else
		tab = read_and_match(dir, dir_path, file_pat, tab);
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
