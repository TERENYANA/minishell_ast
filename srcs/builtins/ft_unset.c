/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_unset.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:15:10 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:15:10 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Prints an error message for an invalid option passed to unset.
 */
static int	unset_opt_err(char *arg)
{
	ft_putstr_fd("minishell: unset: `", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd("': invalid option\n", 2);
	return (2);
}

/*
 * Prints an error message for an invalid identifier in an unset argument.
 */
static int	unset_name_err(char *arg)
{
	ft_putstr_fd("minishell: unset: `", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd("': not a valid identifier\n", 2);
	return (0);
}

/*
 * Implements the shell unset builtin: removes variables from the environment.
 */
int	ft_unset(char **args, t_var **env_list)
{
	int	i;
	int	status;

	i = 1;
	status = 0;
	while (args[i])
	{
		if (args[i][0] == '-' && args[i][1] != '\0')
			return (unset_opt_err(args[i]));
		if (valid_name(args[i]) != 0)
			status = unset_name_err(args[i]);
		else
			env_unset(env_list, args[i]);
		i++;
	}
	return (status);
}
