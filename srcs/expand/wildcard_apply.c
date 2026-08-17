#include "../minishell.h"

static int	tab_len(char **tab)
{
	int	i;

	i = 0;
	while (tab && tab[i])
		i++;
	return (i);
}

int	tab_push(char ***tab, const char *name)
{
	int		len;
	char	**new_tab;
	int		i;

	if (!name)
		return (0);
	len = tab_len(*tab);
	new_tab = malloc(sizeof(char *) * (len + 2));
	if (!new_tab)
		return (0);
	i = 0;
	while (i < len)
	{
		new_tab[i] = (*tab)[i];
		i++;
	}
	new_tab[i] = ft_strdup(name);
	if (!new_tab[i])
		return (free(new_tab), 0);
	new_tab[i + 1] = NULL;
	free(*tab);
	*tab = new_tab;
	return (1);
}

int	add_wildcard_args(t_node *node, char *pattern)
{
	char	**matches;
	int		i;

	matches = collect_matches(pattern);
	if (!matches)
		return (free(pattern), 0);
	if (!matches[0])
		return (ft_free_tab(matches), add_arg(node, pattern));
	sort_tab(matches);
	i = 0;
	while (matches[i])
	{
		if (!add_arg(node, ft_strdup(matches[i])))
			return (ft_free_tab(matches), free(pattern), 0);
		i++;
	}
	return (ft_free_tab(matches), free(pattern), 1);
}