/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wildcard.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:48:50 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:48:52 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

int	has_unquoted_star(const char *s)
{
	int		i;
	char	q;

	i = 0;
	q = 0;
	while (s && s[i])
	{
		if (!q && (s[i] == '\'' || s[i] == '"'))
			q = s[i];
		else if (q && s[i] == q)
			q = 0;
		else if (!q && s[i] == '*')
			return (1);
		i++;
	}
	return (0);
}

int	wc_match(const char *pat, const char *s)
{
	while (*pat == '*' && *(pat + 1) == '*')
		pat++;
	if (!*pat)
		return (!*s);
	if (*pat == '*')
	{
		if (wc_match(pat + 1, s))
			return (1);
		if (*s && wc_match(pat, s + 1))
			return (1);
		return (0);
	}
	if (*pat != *s)
		return (0);
	return (wc_match(pat + 1, s + 1));
}
