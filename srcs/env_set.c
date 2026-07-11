#include "minishell.h"

t_var	*find_var(t_var *env, const char *name)
{
	while (env)
	{
		if (ft_strcmp(env->name, name) == 0)
			return (env);
		env = env->next;
	}
	return (NULL);
}

static int	env_append(t_var **env, const char *name, char *value)
{
	t_var	*node;
	t_var	*cur;

	node = ft_calloc(1, sizeof(t_var));
	if (!node)
		return (free(value), 1);
	node->name = ft_strdup(name);
	if (!node->name)
		return (free(node), free(value), 1);
	node->value = value;
	if (!*env)
	{
		*env = node;
		return (0);
	}
	cur = *env;
	while (cur->next)
		cur = cur->next;
	cur->next = node;
	node->prev = cur;
	return (0);
}

int	env_set(t_var **env, const char *name, char *value)
{
	t_var	*found;

	found = find_var(*env, name);
	if (found)
	{
		free(found->value);
		found->value = value;
		return (0);
	}
	return (env_append(env, name, value));
}

static void	env_remove(t_var **env, t_var *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else
		*env = node->next;
	if (node->next)
		node->next->prev = node->prev;
	free(node->name);
	free(node->value);
	free(node);
}

void	env_unset(t_var **env, const char *name)
{
	t_var	*node;

	node = find_var(*env, name);
	if (node)
		env_remove(env, node);
}