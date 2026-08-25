/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_unset.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:37:28 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:37:30 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static int	unset_opt_err(char *arg)
{
	ft_putstr_fd("minishell: unset: `", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd("': invalid option\n", 2);
	return (2);
}

static int	unset_name_err(char *arg)
{
	ft_putstr_fd("minishell: unset: `", 2);
	ft_putstr_fd(arg, 2);
	ft_putstr_fd("': not a valid identifier\n", 2);
	return (0);
}

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
