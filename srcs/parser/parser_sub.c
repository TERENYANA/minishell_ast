#include "../minishell.h"

static int	sub_redirs(t_node *sub, t_token **cur, t_parse_info *info)
{
	while (*cur && is_redir_tok((*cur)->type))
	{
		if (!process_redir(sub, cur, info))
			return (0);
	}
	return (1);
}

t_node	*parse_primary(t_token **cur, t_parse_info *info)
{
	t_node *inner;
	t_node *sub;

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
		syntax_err(*cur ? (*cur)->value : "newline", info->error_code);
		return (NULL);
	}
	*cur = (*cur)->next;
	sub = new_op_node(N_SUB, inner, NULL);
	if (!sub)
	{
		ft_free_node(inner);
		return (NULL);
	}
	if (!sub_redirs(sub, cur, info))
	{
		ft_free_node(sub);
		return (NULL);
	}
	return (sub);
}