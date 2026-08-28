/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:37:01 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:37:03 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	valid_name(char *str)
{
	int	i;

	if (!str || str[0] == '\0')
		return (-1);
	if (!ft_isalpha(str[0]) && str[0] != '_')
		return (-1);
	i = 1;
	while (str[i])
	{
		if (!ft_isalnum(str[i]) && str[i] != '_')
			return (-1);
		i++;
	}
	return (0);
}

void	err_export(char *name)
{
	ft_putstr_fd("minishell: export: '", 2);
	ft_putstr_fd(name, 2);
	ft_putstr_fd("': not a valid identifier\n", 2);
}

int	parse_export_arg(char *arg, char **name, char **value)
{
	char	*equal_pos;

	if (!arg)
		return (1);
	equal_pos = ft_strchr(arg, '=');
	if (!equal_pos)
		*name = ft_strdup(arg);
	else
		*name = ft_substr(arg, 0, equal_pos - arg);
	if (!*name)
		return (1);
	*value = NULL;
	if (equal_pos)
	{
		*value = ft_strdup(equal_pos + 1);
		if (!*value)
			return (free(*name), 1);
	}
	return (0);
}

int	check_option(char *arg, int *end_opt)
{
	if (!*end_opt && arg[0] == '-' && arg[1] != '\0')
	{
		if (arg[1] == '-' && arg[2] == '\0')
		{
			*end_opt = 1;
			return (1);
		}
		ft_putstr_fd("minishell: export: ", 2);
		ft_putstr_fd(arg, 2);
		ft_putendl_fd(": invalid option", 2);
		return (2);
	}
	return (0);
}

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
