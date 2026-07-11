#include "minishell.h"

void	ft_free_tab(char **tab)
{
	int	i;

	if (!tab)
		return ;
	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}

void	ft_free_env(t_var *env)
{
	t_var	*next;

	while (env)
	{
		next = env->next;
		free(env->name);
		free(env->value);
		free(env);
		env = next;
	}
}

