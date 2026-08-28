/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:34:38 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 14:14:40 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_builtin(char *cmd_name)
{
	if (!cmd_name)
		return (0);
	if (ft_strcmp(cmd_name, "echo") == 0 || ft_strcmp(cmd_name, "cd") == 0
		|| ft_strcmp(cmd_name, "pwd") == 0 || ft_strcmp(cmd_name, "export") == 0
		|| ft_strcmp(cmd_name, "unset") == 0
		|| ft_strcmp(cmd_name, "exit") == 0 || ft_strcmp(cmd_name, ":") == 0)
		return (1);
	return (0);
}

int	is_env_builtin(char **cmd)
{
	return (cmd && cmd[0] && ft_strcmp(cmd[0], "env") == 0 && !cmd[1]);
}

int	dispatch_builtin(t_node *node, t_var **env_list, int last_status)
{
	char	**args;
	char	*name;

	if (!node || !node->cmd || !node->cmd[0])
		return (0);
	args = node->cmd;
	name = args[0];
	if (ft_strcmp(name, ":") == 0)
		return (0);
	if (ft_strcmp(name, "echo") == 0)
		return (ft_echo(args));
	if (ft_strcmp(name, "pwd") == 0)
		return (ft_pwd(args));
	if (ft_strcmp(name, "env") == 0)
		return (ft_env(*env_list));
	if (ft_strcmp(name, "export") == 0)
		return (ft_export(args, env_list));
	if (ft_strcmp(name, "cd") == 0)
		return (ft_cd(args, env_list));
	if (ft_strcmp(name, "unset") == 0)
		return (ft_unset(args, env_list));
	if (ft_strcmp(name, "exit") == 0)
		return (ft_exit(node, node, env_list, last_status));
	return (127);
}
