#include "../minishell.h"

// Forward declaration for functions defined in parser_sub.c
t_node	*parse_primary(t_token **cur, t_parse_info *info);

t_node	*parse_command(t_token **cur, t_parse_info *info)
{
	t_node	*node;

	node = new_cmd_node();
	if (!node)
		return (NULL);
	while (*cur && !is_cmd_end(*cur))
	{
		if ((*cur)->type == WORD)
		{
			if (!add_word(node, *cur, info))
				return (syntax_err_node(node, info));
			*cur = (*cur)->next;
		}
		else if (is_redir_tok((*cur)->type))
		{
			if (!process_redir(node, cur, info))
			{
				ft_free_node(node);
				return (NULL);
			}
		}
		else
			return (syntax_err_node(node, info));
	}
	if (!node->cmd && !node->redirect)
		return (syntax_err_node(node, info));
	return (node);
}

t_node	*parse_pipeline(t_token **cur, t_parse_info *info)
{
	t_node	*left;
	t_node	*right;
	t_node	*new_node;

	left = parse_primary(cur, info);
	if (!left)
		return (NULL);
	while (*cur && (*cur)->type == PIPE)
	{
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
		new_node = new_op_node(N_PIPE, left, right);
		if (!new_node)
		{
			ft_free_node(left);
			ft_free_node(right);
			return (NULL);
		}
		left = new_node;
	}
	return (left);
}

t_node	*parse_list(t_token **cur, t_parse_info *info)
{
	t_node		*node;
	t_node_type	type;
	t_node		*right;

	node = parse_pipeline(cur, info);
	if (!node)
		return (NULL);
	while (*cur && ((*cur)->type == AND_IF || (*cur)->type == OR_IF))
	{
		if ((*cur)->type == AND_IF)
			type = N_AND;
		else
			type = N_OR;
		*cur = (*cur)->next;
		if (!*cur)
		{
			if (info->error_code)
				*info->error_code = 2;
			ft_free_node(node);
			return (NULL);
		}
		right = parse_pipeline(cur, info);
		if (!right)
		{
			ft_free_node(node);
			return (NULL);
		}
		node = new_op_node(type, node, right);
	}
	return (node);
}

t_node	*parsing(t_token *head, t_var *env, int status, int *error_code)
{
	t_parse_info	info;
	t_token			*cur;
	t_node			*tree;

	info.env = env;
	info.status = status;
	info.error_code = error_code;
	if (error_code)
		*error_code = 0;
	if (!head)
		return (NULL);
	cur = head;
	tree = parse_list(&cur, &info);
	if (cur != NULL || (info.error_code && *info.error_code != 0))
	{
		if (error_code && *error_code == 0)
			*error_code = 2;
		if (tree)
			ft_free_node(tree);
		return (NULL);
	}
	return (tree);
}