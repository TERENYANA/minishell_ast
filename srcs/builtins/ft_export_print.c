/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export_print.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:36:44 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:36:47 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static int	count_vars(t_var *env)
{
	int	n;

	n = 0;
	while (env)
	{
		n++;
		env = env->next;
	}
	return (n);
}

static void	print_sorted(t_var **arr, int n)
{
	int	i;

	i = -1;
	while (++i < n)
	{
		if (arr[i]->value)
			printf("declare -x %s=\"%s\"\n", arr[i]->name, arr[i]->value);
		else
			printf("declare -x %s\n", arr[i]->name);
	}
}

int	print_export(t_var *env)
{
	t_var	**arr;
	int		n;
	int		i;

	n = count_vars(env);
	if (n == 0)
		return (0);
	arr = malloc(sizeof(t_var *) * n);
	if (!arr)
		return (1);
	i = 0;
	while (env)
	{
		arr[i++] = env;
		env = env->next;
	}
	sort_ptrs(arr, n);
	print_sorted(arr, n);
	return (free(arr), 0);
}
