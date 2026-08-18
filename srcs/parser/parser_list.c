/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_list.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:12:10 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 16:12:12 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static t_node	*list_rhs(t_node *left, t_token **cur, t_parse_info *info)
{
	t_node		*right;
	t_node_type	type;

	type = N_OR;
	if ((*cur)->type == AND_IF)
		type = N_AND;
	*cur = (*cur)->next;
	if (!*cur)
	{
		if (info->error_code)
			*info->error_code = 2;
		return (ft_free_node(left), NULL);
	}
	right = parse_pipeline(cur, info);
	if (!right)
		return (ft_free_node(left), NULL);
	return (new_op_node(type, left, right));
}

t_node	*parse_list(t_token **cur, t_parse_info *info)
{
	t_node	*node;

	node = parse_pipeline(cur, info);
	if (!node)
		return (NULL);
	while (*cur && ((*cur)->type == AND_IF || (*cur)->type == OR_IF))
	{
		node = list_rhs(node, cur, info);
		if (!node)
			return (NULL);
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
	if (cur != NULL || (error_code && *error_code != 0))
	{
		if (error_code && *error_code == 0)
			*error_code = 2;
		ft_free_node(tree);
		return (NULL);
	}
	return (tree);
}
