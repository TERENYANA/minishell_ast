/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export_print.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:36:44 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/29 15:28:32 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Counts exported variables while skipping the special _ entry.
 */
static int	count_vars(t_var *env)
{
	int	n;

	n = 0;
	while (env)
	{
		if (ft_strcmp(env->name, "_") != 0)
			n++;
		env = env->next;
	}
	return (n);
}

/*
 * Prints the sorted environment list in the export format, with or without a value.
 */
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

/*
 * Sorts an array of variable pointers alphabetically by name.
 */
void	sort_ptrs(t_var **arr, int n)
{
	int		i;
	int		j;
	t_var	*tmp;

	i = 0;
	while (i < n - 1)
	{
		j = 0;
		while (j < n - 1 - i)
		{
			if (ft_strcmp(arr[j]->name, arr[j + 1]->name) > 0)
			{
				tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
			}
			j++;
		}
		i++;
	}
}

/*
 * Prints all environment variables in sorted export format.
 */
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
		if (ft_strcmp(env->name, "_") != 0)
			arr[i++] = env;
		env = env->next;
	}
	sort_ptrs(arr, n);
	print_sorted(arr, n);
	free(arr);
	return (0);
}
