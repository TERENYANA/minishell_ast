/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_escape.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/20 15:51:10 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/20 17:02:00 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	handle_escape(char **res, char *s, int *i, t_qstate q)
{
	if (s[*i] == '\\' && s[*i + 1])
	{
		if ((q == Q_DOUBLE && ft_strchr("$\"\\`", s[*i + 1]))
			|| q == Q_NONE)
		{
			(*i)++;
			*res = append_char(*res, s[(*i)++]);
			return (1);
		}
	}
	return (0);
}
