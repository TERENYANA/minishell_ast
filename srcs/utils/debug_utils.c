#include "../minishell.h"

void	print_env(t_var *e)
{
	while (e)
	{
		if (e->value)
			printf("%s = %s\n", e->name, e->value);
		else
			printf("%s = (unset)\n", e->name);
		e = e->next;
	}
}
