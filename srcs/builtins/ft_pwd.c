/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pwd.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:37:15 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 19:17:26 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

#include "minishell.h"

int	ft_pwd(char **args, t_var *env)
{
	char	*pwd;

	(void)args;
	pwd = get_env_value("PWD", env);
	if (pwd)
	{
		ft_putendl_fd(pwd, STDOUT_FILENO);
		return (0);
	}
	pwd = getcwd(NULL, 0);
	if (!pwd)
	{
		perror("pwd");
		return (1);
	}
	ft_putendl_fd(pwd, STDOUT_FILENO);
	free(pwd);
	return (0);
}
