/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_get.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:38:41 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 14:14:37 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Looks up a variable by name in the shell's t_var list and returns
** its value (or NULL if not found, or if found but has no value --
** e.g. a variable that was `export`ed without an '=', like '_' after
** create_env, or an unset-then-re-exported name). This is the single
** read path used everywhere the shell needs a variable's value:
** $VAR expansion, PATH lookup, HOME/OLDPWD for cd, etc.
*/
char	*get_env_value(char *name, t_var *env)
{
	while (env)
	{
		if (ft_strcmp(env->name, name) == 0)
			return (env->value);
		env = env->next;
	}
	return (NULL);
}

/*
** Counts how many nodes in the list actually have a value set.
** Variables with value == NULL (exported but never assigned, e.g.
** `export FOO` with no '=') exist in the shell's own bookkeeping
** but must never appear in the envp array handed to execve --
** a real OS environment has no concept of a "name-only" entry.
** This sizes the array convert_env_list is about to allocate.
*/
static int	count_valued(t_var *env)
{
	int	n;

	n = 0;
	while (env)
	{
		if (env->value)
			n++;
		env = env->next;
	}
	return (n);
}

/*
** Builds one "NAME=VALUE" string for a single t_var node, the exact
** format execve's envp array expects. Frees the intermediate
** "NAME=" string itself; the caller owns the final joined string.
*/
static char	*env_join(t_var *v)
{
	char	*tmp;
	char	*res;

	tmp = ft_strjoin(v->name, "=");
	if (!tmp)
		return (NULL);
	res = ft_strjoin(tmp, v->value);
	free(tmp);
	return (res);
}

/*
** Converts the shell's internal t_var list into a char** array in
** the "NAME=VALUE" format execve requires. Only nodes with a value
** are included (see count_valued) -- this is the bridge between the
** shell's own environment representation and what the OS actually
** needs when launching an external command. NULL-terminated, as
** every envp array must be. On any allocation failure partway
** through, the partially-built array is freed and NULL returned so
** the caller never gets a half-filled, unterminated array.
*/
char	**convert_env_list(t_var *env)
{
	char	**arr;
	int		i;

	arr = ft_calloc(count_valued(env) + 1, sizeof(char *));
	if (!arr)
		return (NULL);
	i = 0;
	while (env)
	{
		if (env->value)
		{
			arr[i] = env_join(env);
			if (!arr[i])
				return (ft_free_tab(arr), NULL);
			i++;
		}
		env = env->next;
	}
	return (arr);
}
