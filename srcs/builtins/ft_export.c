/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:36:19 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:36:21 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static int	handle_export_arg(char *arg, t_var **env_list)
{
	char	*name;
	char	*value;

	if (parse_export_arg(arg, &name, &value))
		return (1);
	if (valid_name(name))
	{
		err_export(arg);
		free(name);
		free(value);
		return (1);
	}
	env_set(env_list, name, value);
	free(name);
	return (0);
}

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

int	ft_export(char **args, t_var **env_list)
{
	int	end_opt;

	if (!args[1])
		return (print_export(*env_list));
	end_opt = 0;
	return (export_loop(args, env_list, &end_opt));
}
