#include "minishell.h"

void print_env(t_var *e)
{
    while (e)
    {
        if (e->value)
        {
            printf("%s = %s\n", e->name, e->value);
        }
        else
        {
            printf("%s = (unset)\n", e->name);
        }
        e = e->next;
    }
}

void	print_tokens(t_token *tokens)
{
	int	i;

	i = 0;
	printf("\n=== [DEBUG] TOKEN LIST ===\n");
	if (!tokens)
	{
		printf("(empty list)\n");
		return ;
	}
	while (tokens)
	{
		printf("Token [%d]: \"%s\"\n", i, tokens->value ? tokens->value : "(null)");
		/* Если у вас в структуре t_token есть поле типа (type), 
		   можно раскомментировать строку ниже для вывода типа: */
		// printf("  -> Type: %d\n", tokens->type);
		
		tokens = tokens->next;
		i++;
	}
	printf("==========================\n\n");
}