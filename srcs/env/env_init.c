/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:39:07 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/29 15:02:42 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Splits one raw envp line ("NAME=VALUE" or just "NAME") into its
** name and value parts. If there's no '=', value is left NULL
** (matches new_var's old behavior for a name-only entry).
*/
static void	parse_line(char *envp_line, char **name, char **value)
{
	char	*eq;

	eq = ft_strchr(envp_line, '=');
	if (eq)
	{
		*name = ft_substr(envp_line, 0, eq - envp_line);
		*value = ft_strdup(eq + 1);
	}
	else
	{
		*name = ft_strdup(envp_line);
		*value = NULL;
	}
}

t_var	*new_var(char *envp_line)
{
	t_var	*node;
	char	*eq;

	node = ft_calloc(1, sizeof(t_var));
	if (!node)
		return (NULL);
	eq = ft_strchr(envp_line, '=');
	if (eq)
	{
		node->name = ft_substr(envp_line, 0, eq - envp_line);
		node->value = ft_strdup(eq + 1);
		if (!node->name || !node->value)
			return (free(node->name), free(node->value), free(node), NULL);
	}
	else
	{
		node->name = ft_strdup(envp_line);
		if (!node->name)
			return (free(node), NULL);
	}
	return (node);
}

/*
** Builds the shell's t_var environment list from the envp array
** handed to main() by the OS. Uses env_set (not a raw append) for
** each line so that if envp ever contains the same variable name
** twice, the second occurrence UPDATES the existing node instead
** of creating a duplicate -- matching real shell semantics, where
** the last assignment of a repeated name wins and env/export never
** show the same variable more than once.
*/
t_var	*create_env(char **envp)
{
	t_var	*env;
	char	*name;
	char	*value;
	int		i;

	env = NULL;
	i = 0;
	while (envp && envp[i])
	{
		parse_line(envp[i], &name, &value);
		if (!name)
			return (free(value), ft_free_env(env), NULL);
		if (env_set(&env, name, value))
			return (free(name), ft_free_env(env), NULL);
		free(name);
		i++;
	}
	return (env);
}