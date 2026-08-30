/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_nodes.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:12:22 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 14:11:15 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** The two constructors for every node the parser ever creates.
** new_cmd_node makes a leaf (an actual command, with cmd/redirect
** filled in later); new_op_node makes a connector (N_PIPE, N_AND,
** N_OR, N_SUB) that just holds pointers to two already-built
** subtrees. Every other parser file calls these instead of
** allocating t_node directly, so node creation and its failure
** handling live in exactly one place.
*/

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
