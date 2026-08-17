#include "../minishell.h"

int	ft_env(t_var *env)
{
	if (!env)
		return (0);
	while (env)
	{
		if (env->value)
			printf("%s=%s\n", env->name, env->value);
		env = env->next;
	}
	return (0);
}
