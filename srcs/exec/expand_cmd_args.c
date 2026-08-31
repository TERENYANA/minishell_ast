/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_cmd_args.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 17:15:27 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/31 17:19:14 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	expand_cmd_args(t_node *cmd_node, t_var *env, int status)
{
	char	**old_cmd;
	char	*expanded;
	int		i;

	if (!cmd_node || !cmd_node->cmd)
		return ;
	old_cmd = cmd_node->cmd;
	cmd_node->cmd = NULL;
	i = 0;
	while (old_cmd[i])
	{
		expanded = ft_expand(old_cmd[i], env, status);
		if (expanded && expanded[0] == '\0' && !has_quotes(old_cmd[i]))
			free(expanded);
		else if (expanded && has_unquoted_star(old_cmd[i]))
			add_wildcard_args(cmd_node, expanded);
		else if (expanded)
			add_arg(cmd_node, expanded);
		i++;
	}
	ft_free_tab(old_cmd);
}
