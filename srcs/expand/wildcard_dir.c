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
	return (wc_match(pat, name));
}

char	**collect_matches(const char *pattern)
{
	DIR				*dir;
	struct dirent	*e;
	char			**tab;

	tab = ft_calloc(1, sizeof(char *));
	dir = opendir(".");
	if (!tab || !dir)
		return (ft_free_tab(tab), NULL);
	e = readdir(dir);
	while (e)
	{
		if (visible_match(pattern, e->d_name) && !tab_push(&tab, e->d_name))
			return (closedir(dir), ft_free_tab(tab), NULL);
		e = readdir(dir);
	}
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
