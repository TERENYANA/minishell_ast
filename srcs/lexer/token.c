#include "../minishell.h"

t_token	*create_token(char *value, t_token_type type)
{
	t_token	*t;

	t = malloc(sizeof(t_token));
	if (!t)
		return (NULL);
	t->value = value;
	t->type = type;
	t->next = NULL;
	return (t);
}

bool	add_token(char *value, t_token_type type, t_tok_list *list)
{
	t_token	*t;

	t = create_token(value, type);
	if (!t)
		return (false);
	if (!list->head)
		list->head = t;
	else
		list->tail->next = t;
	list->tail = t;
	return (true);
}

t_token_type	assign_type(const char *s)
{
	if (s[0] == '&' && s[1] == '&')
		return (AND_IF);
	if (s[0] == '|' && s[1] == '|')      /* ДО одиночного '|' ! */
		return (OR_IF);
	if (s[0] == '|')
		return (PIPE);
	if (s[0] == '<' && s[1] == '<')
		return (HEREDOC);
	if (s[0] == '>' && s[1] == '>')
		return (REDIR_APPEND);
	if (s[0] == '<')
		return (REDIR_IN);
	if (s[0] == '>')
		return (REDIR_OUT);
	if (s[0] == '(')
		return (LPAREN);
	if (s[0] == ')')
		return (RPAREN);
	return (WORD);
}

char	*extract_quoted(char *s, int len)
{
	char	*res;

	res = malloc(len + 1);
	if (!res)
		return (NULL);
	ft_memcpy(res, s, len);
	res[len] = '\0';
	return (res);
}
