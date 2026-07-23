#include "../minishell.h"

static int	is_logic(t_token_type t)
{
	return (t == PIPE || t == AND_IF || t == OR_IF);
}

int	syntax_err(const char *tok, int *error_code)
{
	ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
	ft_putstr_fd((char *)tok, 2);
	ft_putendl_fd("'", 2);
	if (error_code)
		*error_code = 2;
	return (0);
}

/* Equilibre des parentheses + interdiction de "()" vide. */
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
		if (t->type == RPAREN && --depth < 0)
			return (syntax_err(")", error_code));
		t = t->next;
	}
	if (depth > 0)
		return (syntax_err("(", error_code));
	return (1);
}

static int	check_operators(t_token *t, int *error_code)
{
	while (t)
	{
		if (is_logic(t->type) && (!t->next || is_logic(t->next->type)
				|| t->next->type == RPAREN))
			return (syntax_err(t->next ? t->next->value : t->value,
					error_code));
		if (is_redir_tok(t->type) && (!t->next || t->next->type != WORD))
			return (syntax_err(t->next ? t->next->value : "newline",
					error_code));
		t = t->next;
	}
	return (1);
}

int	syntax_ok(t_token *t, int *error_code)
{
	if (error_code)
		*error_code = 0;
	if (t && is_logic(t->type))
		return (syntax_err(t->value, error_code));
	if (!check_operators(t, error_code))
		return (0);
	return (check_parens(t, error_code));
}