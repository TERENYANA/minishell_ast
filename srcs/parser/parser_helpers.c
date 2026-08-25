/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:11:49 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 16:11:51 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

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
	char	*expanded;

	if (!tok || !tok->value)
		return (1);
	expanded = ft_expand(tok->value, info->env, info->status);
	if (!expanded)
		return (0);
	if (expanded[0] == '\0' && !has_quotes(tok->value))
		return (free(expanded), 1);
	if (has_unquoted_star(tok->value))
		return (add_wildcard_args(node, expanded));
	return (add_arg(node, expanded));
}

int	is_cmd_end(t_token *t)
{
	if (!t)
		return (1);
	return (t->type == PIPE || t->type == AND_IF
		|| t->type == OR_IF || t->type == RPAREN);
}
