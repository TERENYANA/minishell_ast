/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:37:01 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/29 19:57:18 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Validates that an export variable name is syntactically valid.
 */
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

/*
 * Prints the standard error message when an export identifier is invalid.
 */
void	err_export(char *name)
{
	ft_putstr_fd("minishell: export: '", 2);
	ft_putstr_fd(name, 2);
	ft_putstr_fd("': not a valid identifier\n", 2);
}

/*
 * Extracts the value part of an export assignment, if one is present.
 */
static int	extract_value(char *equal_pos, char **name, char **value)
{
	*value = NULL;
	if (equal_pos)
	{
		*value = ft_strdup(equal_pos + 1);
		if (!*value)
		{
			free(*name);
			return (1);
		}
	}
	return (0);
}

/*
 * Parses an export argument into a variable name and an optional value,
 * while detecting appending syntax such as NAME+=value.
 */
int	parse_export_arg(char *arg, char **name, char **value, int *is_append)
{
	char	*equal_pos;
	int		name_len;

	if (!arg)
		return (1);
	*is_append = 0;
	equal_pos = ft_strchr(arg, '=');
	if (equal_pos)
		name_len = equal_pos - arg;
	else
		name_len = ft_strlen(arg);
	if (equal_pos && name_len > 0 && arg[name_len - 1] == '+')
	{
		*is_append = 1;
		name_len--;
	}
	*name = ft_substr(arg, 0, name_len);
	if (!*name)
		return (1);
	return (extract_value(equal_pos, name, value));
}

/*
 * Checks whether an option token is provided to 
 * export and handles invalid usage.
 */
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
