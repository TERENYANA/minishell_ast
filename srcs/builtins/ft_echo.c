/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_echo.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:35:13 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:35:15 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static int	is_n_flag(char *arg)
{
	int	j;

	if (!arg || arg[0] != '-' || arg[1] != 'n')
		return (0);
	j = 2;
	while (arg[j] == 'n')
		j++;
	return (arg[j] == '\0');
}

int	ft_echo(char **args)
{
	int	i;
	int	newline;
	char	pid_msg[100];
	sprintf(pid_msg, "DEBUG: ft_echo called in PID %d\n", getpid());
	ft_putstr_fd(pid_msg, 2);
	i = 1;
	newline = 1;
	while (args[i] && is_n_flag(args[i]))
	{
		newline = 0;
		i++;
	}
	while (args[i])
	{
		write(STDOUT_FILENO, args[i], ft_strlen(args[i]));
		if (args[i + 1])
			write(STDOUT_FILENO, " ", 1);
		i++;
	}
	if (newline)
		write(STDOUT_FILENO, "\n", 1);
	return (0);
}
