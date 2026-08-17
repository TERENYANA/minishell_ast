#include "../minishell.h"

static int	cmd_step(t_node *node, t_token **cur, t_parse_info *info)
{
	if ((*cur)->type == WORD)
	{
		if (!add_word(node, *cur, info))
		{
			if (info && info->error_code)
				*info->error_code = 2;
			return (0);
		}
		*cur = (*cur)->next;
		return (1);
	}
	if (is_redir_tok((*cur)->type))
		return (process_redir(node, cur, info));
	if (info && info->error_code)
		*info->error_code = 2;
	return (0);
}

t_node	*parse_command(t_token **cur, t_parse_info *info)
{
	t_node	*node;
	int		seen;

	node = new_cmd_node();
	if (!node)
		return (NULL);
	seen = 0;
	while (*cur && !is_cmd_end(*cur))
	{
		if (!cmd_step(node, cur, info))
		{
			ft_free_node(node);
			return (NULL);
		}
		seen = 1;
	}
	if (!node->cmd && !node->redirect && !seen)
		return (syntax_err_node(node, info));
	return (node);
}

static t_node	*pipe_rhs(t_node *left, t_token **cur, t_parse_info *info)
{
	t_node	*right;

	*cur = (*cur)->next;
	if (!*cur)
	{
		if (info->error_code)
			*info->error_code = 2;
		return (ft_free_node(left), NULL);
	}
	right = parse_primary(cur, info);
	if (!right)
		return (ft_free_node(left), NULL);
	return (new_op_node(N_PIPE, left, right));
}

t_node	*parse_pipeline(t_token **cur, t_parse_info *info)
{
	t_node	*left;

	left = parse_primary(cur, info);
	if (!left)
		return (NULL);
	while (*cur && (*cur)->type == PIPE)
	{
		left = pipe_rhs(left, cur, info);
		if (!left)
			return (NULL);
	}
	return (left);
}