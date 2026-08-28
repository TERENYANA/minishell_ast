/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:39:07 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 19:10:15 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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

static void	append_var(t_var **head, t_var **tail, t_var *node)
{
	if (!*head)
	{
		*head = node;
		*tail = node;
		return ;
	}
	node->prev = *tail;
	(*tail)->next = node;
	*tail = node;
}
t_var	*create_env(char **envp)
{
	t_var	*head;
	t_var	*tail;
	t_var	*node;
	int		i;

	head = NULL;
	tail = NULL;
	i = 0;
	while (envp && envp[i])
	{
		node = new_var(envp[i]);
		if (!node)
			return (ft_free_env(head), NULL);
		append_var(&head, &tail, node);
		i++;
	}
	return (head);
}
