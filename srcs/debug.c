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
		/* Si vous avez un champ de type (type) dans la structure t_token, 
		   vous pouvez décommenter la ligne ci-dessous pour afficher le type : */
		// printf("  -> Type: %d\n", tokens->type);
		
		tokens = tokens->next;
		i++;
	}
	printf("==========================\n\n");
}