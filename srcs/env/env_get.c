#include "../minishell.h"

char	*get_env_value(char *name, t_var *env)
{
	while (env)
	{
		if (ft_strcmp(env->name, name) == 0)
			return (env->value);
		env = env->next;
	}
	return (NULL);
}

static int	count_valued(t_var *env)
{
	int	n;

	n = 0;
	while (env)
	{
		if (env->value)
			n++;
		env = env->next;
	}
	return (n);
}

static char	*env_join(t_var *v)
{
	char	*tmp;
	char	*res;

	tmp = ft_strjoin(v->name, "=");
	if (!tmp)
		return (NULL);
	res = ft_strjoin(tmp, v->value);
	free(tmp);
	return (res);
}

char	**convert_env_list(t_var *env)
{
	char **arr;
	int i;

	// Nous allouons de la mémoire et la remplissons immédiatement avec des zéros (\0/ NULL)
	arr = ft_calloc(count_valued(env) + 1, sizeof(char *));
	if (!arr)
		return (NULL);
	i = 0;
	while (env)
	{
		if (env->value)
		{
			arr[i] = env_join(env);
			if (!arr[i])
				return (ft_free_tab(arr), NULL);
			i++;
		}
		env = env->next;
	}
	// car ft_calloc a déjà initialisé tout le tableau à zéro.
	return (arr);
}