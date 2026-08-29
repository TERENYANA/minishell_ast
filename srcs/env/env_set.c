/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_set.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:39:24 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 14:14:33 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Finds and returns the t_var node matching `name`, or NULL if no
** such variable exists. Unlike get_env_value, this returns the
** NODE itself (not just its value), so callers can modify it in
** place (env_set) or unlink it (env_unset).
*/
t_var	*find_var(t_var *env, const char *name)
{
	while (env)
	{
		if (ft_strcmp(env->name, name) == 0)
			return (env);
		env = env->next;
	}
	return (NULL);
}

/*
** Allocates a brand-new node and appends it to the end of the list
** . Only called by
** env_set when find_var confirmed no existing node has this name,
** so this never creates a duplicate on its own.
** Takes ownership of `value` (stores the pointer as-is, no copy);
** `name` is copied via ft_strdup since the caller may still need
** its own copy afterwards (create_env frees it right after).
*/
static int	env_append(t_var **env, const char *name, char *value)
{
	t_var	*node;
	t_var	*cur;

	node = ft_calloc(1, sizeof(t_var));
	if (!node)
		return (free(value), 1);
	node->name = ft_strdup(name);
	if (!node->name)
		return (free(node), free(value), 1);
	node->value = value;
	if (!*env)
	{
		*env = node;
		return (0);
	}
	cur = *env;
	while (cur->next)
		cur = cur->next;
	cur->next = node;
	node->prev = cur;
	return (0);
}

/*
** Single entry point for setting/updating an environment variable.
** If `name` already exists, its old value is freed and replaced
** in place (the node itself, and its position in the list, never
** change) -- this is what keeps env/export from ever showing the
** same variable twice, no matter how many times it's assigned.
** If `name` doesn't exist yet, delegates to env_append to create it.
** Takes ownership of `value` either way (freed here on update,
** or stored/freed inside env_append on insert).
*/
int	env_set(t_var **env, const char *name, char *value)
{
	t_var	*found;

	found = find_var(*env, name);
	if (found)
	{
		free(found->value);
		found->value = value;
		return (0);
	}
	return (env_append(env, name, value));
}

/*
** Unlinks one node from the doubly-linked list, fixing up its
** neighbors' prev/next pointers (including the head-of-list case,
** where node->prev is NULL), then frees the node's owned strings
** and the node itself. Only called from env_unset once the node
** to remove has already been found.
*/
static void	env_remove(t_var **env, t_var *node)
{
	if (node->prev)
		node->prev->next = node->next;
	else
		*env = node->next;
	if (node->next)
		node->next->prev = node->prev;
	free(node->name);
	free(node->value);
	free(node);
}

/*
** Removes a variable from the environment by name (used by the
** `unset` builtin). Silently does nothing if the name isn't found --
** unset on a non-existent variable is not an error in bash either.
*/
void	env_unset(t_var **env, const char *name)
{
	t_var	*node;

	node = find_var(*env, name);
	if (node)
		env_remove(env, node);
}