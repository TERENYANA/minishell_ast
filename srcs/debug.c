#include "minishell.h"

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

static const char	*tt(t_token_type t)
{
	if (t == WORD)
		return ("WORD");
	if (t == PIPE)
		return ("PIPE");
	if (t == REDIR_IN)
		return ("REDIR_IN");
	if (t == REDIR_OUT)
		return ("REDIR_OUT");
	if (t == REDIR_APPEND)
		return ("REDIR_APPEND");
	if (t == HEREDOC)
		return ("HEREDOC");
	if (t == AND_IF)
		return ("AND_IF");
	if (t == OR_IF)
		return ("OR_IF");
	if (t == LPAREN)
		return ("LPAREN");
	return ("RPAREN");
}

void	print_tokens(t_token *tokens)
{
	if (!tokens)
		return ((void)printf("(empty list)\n"));
	while (tokens)
	{
		printf("[%s: '%s'] ", tt(tokens->type),
			tokens->value ? tokens->value : "(null)");
		tokens = tokens->next;
	}
	printf("\n");
}

static const char	*nt(t_node_type t)
{
	if (t == N_PIPE)
		return ("N_PIPE");
	if (t == N_AND)
		return ("N_AND");
	if (t == N_OR)
		return ("N_OR");
	return ("N_SUB");
}

static void	print_cmd(t_node *n)
{
	t_redirect	*r;
	int			i;

	printf("[N_CMD] argv=");
	i = 0;
	while (n->cmd && n->cmd[i])
		printf("[%s]", n->cmd[i++]);
	r = n->redirect;
	if (r)
		printf(" redirs=");
	while (r)
	{
		printf("[%s:'%s']", tt(r->type), r->target);
		r = r->next;
	}
	printf("\n");
}

void	print_tree(t_node *n, int d)
{
	int	i;

	i = 0;
	while (i++ < d)
		printf("  ");
	if (!n)
		return ((void)printf("(null)\n"));
	if (n->type == N_CMD)
		return (print_cmd(n));
	printf("[%s]\n", nt(n->type));
	print_tree(n->left, d + 1);
	if (n->right)
		print_tree(n->right, d + 1);
}