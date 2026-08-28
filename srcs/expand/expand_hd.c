/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_hd.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:48:16 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 15:56:14 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*hd_append(char *res, char c)
{
	int		len;
	char	*new;

	if (!res)
		return (NULL);
	len = ft_strlen(res);
	new = malloc(len + 2);
	if (!new)
		return (free(res), NULL);
	ft_memcpy(new, res, len);
	new[len] = c;
	new[len + 1] = '\0';
	free(res);
	return (new);
}

static char	*hd_join_free(char *a, char *b)
{
	char	*res;

	if (!a || !b)
		return (free(a), free(b), NULL);
	res = ft_strjoin(a, b);
	free(a);
	free(b);
	return (res);
}

static char	*hd_dollar(char *s, int *i, t_var *env, int status)
{
	int		start;
	char	*name;
	char	*val;

	if (s[*i + 1] == '?')
		return (*i += 2, ft_itoa(status));
	if (!s[*i + 1] || (!ft_isalpha(s[*i + 1]) && s[*i + 1] != '_'))
		return ((*i)++, ft_strdup("$"));
	start = ++(*i);
	while (s[*i] && (ft_isalnum(s[*i]) || s[*i] == '_'))
		(*i)++;
	name = ft_substr(s, start, *i - start);
	if (!name)
		return (NULL);
	val = get_env_value(name, env);
	free(name);
	if (val)
		return (ft_strdup(val));
	return (ft_strdup(""));
}

char	*expand_heredoc_line(char *s, t_var *env, int status)
{
	char	*res;
	int		i;

	if (!s)
		return (NULL);
	res = ft_strdup("");
	i = 0;
	while (res && s[i])
	{
		if (s[i] == '$')
			res = hd_join_free(res, hd_dollar(s, &i, env, status));
		else
			res = hd_append(res, s[i++]);
	}
	return (res);
}
