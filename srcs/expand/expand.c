#include "../minishell.h"

typedef enum e_qstate { Q_NONE, Q_SINGLE, Q_DOUBLE }	t_qstate;

static char	*append_char(char *res, char c)
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

/*
** ИСПРАВЛЕНО:
** 1. Обработка $? (возврат exit status).
** 2. Обработка $1..$9 (съедает цифру и возвращает пустую строку, т.к. позиционных аргументов в minishell нет).
** 3. Обработка одиночного $ или $ перед кавычками/пробелами.
*/
static char	*expand_dollar(char *s, int *i, t_var *env, int status)
{
	int		start;
	char	*name;
	char	*val;

	if (s[*i + 1] == '?')
		return (*i += 2, ft_itoa(status));
	if (ft_isdigit(s[*i + 1]))
		return (*i += 2, ft_strdup(""));
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
	return (ft_strdup(val ? val : ""));
}

static char	*expand_step(char *s, int *i, t_qstate *q)
{
	if (*q != Q_DOUBLE && s[*i] == '\'')
	{
		*q = (*q == Q_SINGLE) ? Q_NONE : Q_SINGLE;
		(*i)++;
		return (ft_strdup(""));
	}
	if (*q != Q_SINGLE && s[*i] == '"')
	{
		*q = (*q == Q_DOUBLE) ? Q_NONE : Q_DOUBLE;
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
		else if (s[i] == '$' && q != Q_SINGLE)
			res = join_free(res, expand_dollar(s, &i, env, status));
		else
			res = append_char(res, s[i++]);
	}
	return (res);
}

