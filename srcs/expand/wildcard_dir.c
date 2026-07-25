#include "../minishell.h"
#include <dirent.h>

static int	visible_match(const char *pat, const char *name)
{
	if (name[0] == '.' && pat[0] != '.') /* cachés — seulement explicitement */
		return (0);
	return (wc_match(pat, name));
}

/* Collecte toutes les correspondances depuis "." dans un tableau terminé par NULL. */
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

/* Tri à bulles via ft_strcmp — bash trie par ordre alphabétique. */
void	sort_tab(char **tab)
{
	int i;
	char *tmp;

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