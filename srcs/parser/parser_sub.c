#include "../minishell.h"

static t_node	*wrap_sub(t_node *inner, t_token **cur, t_parse_info *info)
{
	t_node	*sub;

	sub = new_op_node(N_SUB, inner, NULL);
	if (!sub)
		return (NULL);
	while (*cur && is_redir_tok((*cur)->type))
	{
		if (!process_redir(sub, cur, info))
			return (ft_free_node(sub), NULL);
	}
	return (sub);
}

t_node	*parse_primary(t_token **cur, t_parse_info *info)
{
	t_node	*inner;

	if (!*cur)
		return (NULL);
	if ((*cur)->type != LPAREN)
		return (parse_command(cur, info));
	*cur = (*cur)->next;
	inner = parse_list(cur, info);
	if (!inner)
		return (NULL);
	if (!*cur || (*cur)->type != RPAREN)
	{
		ft_free_node(inner);
		if (*cur)
			syntax_err((*cur)->value, info->error_code);
		else
			syntax_err("newline", info->error_code);
		return (NULL);
	}
	*cur = (*cur)->next;
	return (wrap_sub(inner, cur, info));
}