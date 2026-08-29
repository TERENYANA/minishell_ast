/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:15:04 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:15:04 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Appends a value to an existing environment variable when the export
 * argument uses the += form.
 */
static int	handle_append(t_var **env_list, char *name, char *value)
{
	t_var	*existing;
	char	*joined;

	existing = find_var(*env_list, name);
	if (existing && existing->value && value)
	{
		joined = ft_strjoin(existing->value, value);
		free(value);
		value = joined;
	}
	if (value)
		env_set(env_list, name, value);
	free(name);
	return (0);
}

/*
 * Validates and applies a single export argument to the shell environment.
 */
static int	handle_export_arg(char *arg, t_var **env_list)
{
	char	*name;
	char	*value;
	int		is_append;

	if (parse_export_arg(arg, &name, &value, &is_append))
		return (1);
	if (valid_name(name))
	{
		err_export(arg);
		free(name);
		free(value);
		return (1);
	}
	if (is_append)
		return (handle_append(env_list, name, value));
	if (value || !find_var(*env_list, name))
		env_set(env_list, name, value);
	free(name);
	return (0);
}

/*
 * Iterates over export arguments, stops on the end-of-options marker,
 * and reports any invalid assignment encountered.
 */
static int	export_loop(char **args, t_var **env_list, int *end_opt)
{
	int	i;
	int	error;
	int	ret;

	i = 1;
	error = 0;
	while (args[i])
	{
		ret = check_option(args[i], end_opt);
		if (ret == 2)
			return (2);
		if (ret == 0 && handle_export_arg(args[i], env_list))
			error = 1;
		i++;
	}
	return (error);
}

/*
 * Implements the shell export builtin: prints the environment or updates it.
 */
int	ft_export(char **args, t_var **env_list)
{
	int	end_opt;

	if (!args[1])
		return (print_export(*env_list));
	end_opt = 0;
	return (export_loop(args, env_list, &end_opt));
}
