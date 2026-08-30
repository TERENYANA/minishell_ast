/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:11:37 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:11:37 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Utilities for building up a single N_CMD node's argument list one
** WORD token at a time (add_word/add_arg), plus is_cmd_end, which
** every parsing function uses to answer "have I reached the
** boundary of the current command?" (a pipe, &&, ||, or a closing
** paren). Since cmd is a plain char** with no reserved capacity,
** add_arg reallocates and copies on every call -- fine at the small
** argument counts a shell command has.
*/
t_node	*syntax_err_node(t_node *node, t_parse_info *info)
{
	if (info && info->error_code)
		*info->error_code = 2;
	if (node)
		ft_free_node(node);
	return (NULL);
}

int	add_arg(t_node *node, char *value)
{
	char	**new_cmd;
	int		i;

	if (!node || !value)
		return (free(value), 0);
	i = 0;
	while (node->cmd && node->cmd[i])
		i++;
	new_cmd = ft_calloc(i + 2, sizeof(char *));
	if (!new_cmd)
		return (free(value), 0);
	i = 0;
	while (node->cmd && node->cmd[i])
	{
		new_cmd[i] = node->cmd[i];
		i++;
	}
	new_cmd[i] = value;
	new_cmd[i + 1] = NULL;
	free(node->cmd);
	node->cmd = new_cmd;
	return (1);
}

int	add_word(t_node *node, t_token *tok, t_parse_info *info)
{
	char	*val;

	(void)info;
	if (!tok || !tok->value)
		return (1);
	val = ft_strdup(tok->value);
	if (!val)
		return (0);
	return (add_arg(node, val));
}

int	is_cmd_end(t_token *t)
{
	if (!t)
		return (1);
	return (t->type == PIPE || t->type == AND_IF
		|| t->type == OR_IF || t->type == RPAREN);
}
