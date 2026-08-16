#include "../minishell.h"

static int	is_logic(t_token_type t)
{
	return (t == PIPE || t == AND_IF || t == OR_IF);
}

int	syntax_err(const char *tok, int *error_code)
{
	ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
	if (tok)
		ft_putstr_fd((char *)tok, 2);
	else
		ft_putstr_fd("newline", 2);
	ft_putendl_fd("'", 2);
	if (error_code)
		*error_code = 2;
	return (0);
}

static int	check_parens(t_token *t, int *error_code)
{
	int	depth;

	depth = 0;
	while (t)
	{
		if (t->type == LPAREN)
		{
			if (!t->next || t->next->type == RPAREN)
				return (syntax_err(t->next ? ")" : "(", error_code));
			depth++;
		}
		if (t->type == RPAREN)
		{
			depth--;
			if (depth < 0)
				return (syntax_err(")", error_code));
		}
		t = t->next;
	}
	if (depth > 0)
		return (syntax_err("newline", error_code));
	return (1);
}

static int	bad_neighbour(t_token *t)
{
	if (t->type == WORD && ft_strcmp(t->value, "&") == 0)
		return (1);
	if (t->type == WORD && t->next && t->next->type == LPAREN)
		return (1);
	if (t->type == RPAREN && t->next && t->next->type == WORD)
		return (1);
	return (0);
}

static int	check_operators(t_token *t, int *error_code)
{
	while (t)
	{
		if (bad_neighbour(t))
			return (syntax_err(t->type == WORD && t->next ? "(" : t->value,
					error_code));
		if (is_logic(t->type))
		{
			if (!t->next)
				return (syntax_err("newline", error_code));
			if (is_logic(t->next->type) || t->next->type == RPAREN)
    			return (syntax_err(t->next->value, error_code));
		}
		if (is_redir_tok(t->type))
		{
			if (!t->next)
				return (syntax_err("newline", error_code));
			if (t->next->type != WORD)
				return (syntax_err(t->next->value, error_code));
		}
		t = t->next;
	}
	return (1);
}

int	syntax_ok(t_token *t, int *error_code)
{
	if (error_code)
		*error_code = 0;
	if (!t)
		return (1);
	if (is_logic(t->type))
		return (syntax_err(t->value, error_code));
	if (!check_operators(t, error_code))
		return (0);
	return (check_parens(t, error_code));
}