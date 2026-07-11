#include "../minishell.h"

static int	is_redir(t_token_type t)
{
	return (t == REDIR_IN || t == REDIR_OUT
		|| t == REDIR_APPEND || t == HEREDOC);
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

int	syntax_ok(t_token *t, int *error_code)
{
	if (error_code)
		*error_code = 0;
	if (t && t->type == PIPE)
		return (syntax_err("|", error_code));
	while (t)
	{
		if (t->type == PIPE && (!t->next || t->next->type == PIPE))
			return (syntax_err("|", error_code));
		if (is_redir(t->type) && (!t->next || t->next->type != WORD))
			return (syntax_err(
					t->next ? t->next->value : "newline", error_code));
		t = t->next;
	}
	return (1);
}