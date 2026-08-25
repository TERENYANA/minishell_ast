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
		{
			ft_putstr_fd("declare -x ", STDOUT_FILENO);
			ft_putstr_fd(arr[i]->name, STDOUT_FILENO);
			ft_putstr_fd("=\"", STDOUT_FILENO);
			ft_putstr_fd(arr[i]->value, STDOUT_FILENO);
			ft_putendl_fd("\"", STDOUT_FILENO);
		}
		else
		{
			ft_putstr_fd("declare -x ", STDOUT_FILENO);
			ft_putendl_fd(arr[i]->name, STDOUT_FILENO);
		}
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
