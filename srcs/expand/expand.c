/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:47:58 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/20 17:05:15 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*join_free(char *a, char *b)
{
	char	*res;

	if (!a || !b)
		return (free(a), free(b), NULL);
	res = ft_strjoin(a, b);
	free(a);
	free(b);
	return (res);
}

static char	*expand_special(char *s, int *i, int status, int *braces)
{
	*braces = 0;
	if (s[*i + 1] == '?')
		return (*i += 2, ft_itoa(status));
	if (s[*i + 1] == '0')
		return (*i += 2, ft_strdup("minishell"));
	if (s[*i + 1] == '{')
	{
		*braces = 1;
		(*i)++;
	}
	if (ft_isdigit(s[*i + 1]))
	{
		*i += 2;
		if (*braces && s[*i] == '}')
			(*i)++;
		return (ft_strdup(""));
	}
	if (!s[*i + 1] || (!ft_isalpha(s[*i + 1]) && s[*i + 1] != '_'))
	{
		if (*braces)
			return ((*i)++, ft_strdup("${"));
		return ((*i)++, ft_strdup("$"));
	}
	return (NULL);
}

static char	*expand_dollar(char *s, int *i, t_var *env, int status)
{
	int		start;
	char	*name;
	char	*val;
	int		braces;

	val = expand_special(s, i, status, &braces);
	if (val)
		return (val);
	start = ++(*i);
	while (s[*i] && (ft_isalnum(s[*i]) || s[*i] == '_'))
		(*i)++;
	name = ft_substr(s, start, *i - start);
	if (braces && s[*i] == '}')
		(*i)++;
	if (!name)
		return (NULL);
	val = get_env_value(name, env);
	free(name);
	if (val)
		return (ft_strdup(val));
	return (ft_strdup(""));
}

static char	*expand_step(char *s, int *i, t_qstate *q)
{
	if (*q != Q_DOUBLE && s[*i] == '\'')
	{
		if (*q == Q_SINGLE)
			*q = Q_NONE;
		else
			*q = Q_SINGLE;
		(*i)++;
		return (ft_strdup(""));
	}
	if (*q != Q_SINGLE && s[*i] == '"')
	{
		if (*q == Q_DOUBLE)
			*q = Q_NONE;
		else
			*q = Q_DOUBLE;
		(*i)++;
		return (ft_strdup(""));
	}
	return (NULL);
}

char	*ft_expand(char *s, t_var *env, int status)
{
	t_qstate	q;
	char		*res;
	char		*tmp;
	int			i;

	if (!s)
		return (NULL);
	res = ft_strdup("");
	q = Q_NONE;
	i = 0;
	while (res && s[i])
	{
		tmp = expand_step(s, &i, &q);
		if (tmp)
			res = join_free(res, tmp);
		else if (!handle_escape(&res, s, &i, q))
		{
			if (s[i] == '$' && q != Q_SINGLE)
				res = join_free(res, expand_dollar(s, &i, env, status));
			else
				res = append_char(res, s[i++]);
		}
	}
	return (res);
}
