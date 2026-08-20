/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_exit.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:36:03 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/20 11:40:09 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static int	skip_ws(const char *s, int i)
{
	while (s[i] == ' ' || (s[i] >= 9 && s[i] <= 13))
		i++;
	return (i);
}

static int	parse_sign(const char *s, int *i)
{
	if (s[*i] == '+' || s[*i] == '-')
	{
		if (s[*i] == '-')
			return ((*i)++, -1);
		(*i)++;
	}
	return (1);
}

static int	fits_in_long_long(const char *str, long long *out)
{
	int					i;
	int					sign;
	unsigned long long	res;

	i = skip_ws(str, 0);
	sign = parse_sign(str, &i);
	res = 0;
	if (!ft_isdigit(str[i]))
		return (0);
	while (ft_isdigit(str[i]))
	{
		res = res * 10 + (str[i] - '0');
		if (sign == 1 && res > (unsigned long long)LLONG_MAX)
			return (0);
		if (sign == -1 && res > (unsigned long long)LLONG_MAX + 1ULL)
			return (0);
		i++;
	}
	i = skip_ws(str, i);
	if (str[i] != '\0')
		return (0);
	return (*out = (long long)res * sign, 1);
}

static void	exit_numeric_err(const char *arg)
{
	ft_putstr_fd("minishell: exit: ", 2);
	ft_putstr_fd((char *)arg, 2);
	ft_putendl_fd(": numeric argument required", 2);
}

int	ft_exit(t_node *root, t_node *cur, t_var **env, int last_status)
{
	long long	code;

	if (isatty(STDIN_FILENO))
		ft_putendl_fd("exit", STDOUT_FILENO);
	if (!cur->cmd[1])
		cleanup_and_exit(root, env, last_status);
	if (!fits_in_long_long(cur->cmd[1], &code))
	{
		exit_numeric_err(cur->cmd[1]);
		cleanup_and_exit(root, env, 2);
	}
	if (cur->cmd[2])
		return (err_msg("exit", "too many arguments", 1));
	cleanup_and_exit(root, env, (unsigned char)code);
	return (0);
}
