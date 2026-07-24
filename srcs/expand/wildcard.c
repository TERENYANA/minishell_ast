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

/*
** ИСПРАВЛЕНО: Сворачивание идущих подряд звёздочек ('**' -> '*').
** Это предотвращает глубокую рекурсию и падение по Segfault.
*/
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