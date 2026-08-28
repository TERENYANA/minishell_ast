/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_nodes.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:12:22 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 16:12:24 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_node	*new_cmd_node(void)
{
	t_node	*n;

	n = ft_calloc(1, sizeof(t_node));
	if (!n)
		return (NULL);
	n->type = N_CMD;
	return (n);
}

t_node	*new_op_node(t_node_type type, t_node *left, t_node *right)
{
	t_node	*n;

	n = ft_calloc(1, sizeof(t_node));
	if (!n)
		return (ft_free_node(left), ft_free_node(right), NULL);
	n->type = type;
	n->left = left;
	n->right = right;
	return (n);
}
