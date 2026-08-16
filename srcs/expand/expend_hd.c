#include "../minishell.h"

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
	{
		if (a)
			free(a);
		if (b)
			free(b);
		return (NULL);
	}
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
	{
		*i += 2;
		return (ft_itoa(status));
	}
	if (!s[*i + 1] || (!ft_isalpha(s[*i + 1]) && s[*i + 1] != '_'))
	{
		(*i)++;
		return (ft_strdup("$"));
	}
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

char	*expand_heredoc_line(char *s, t_var *env, int status)
{
	char *res;
	int i;

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