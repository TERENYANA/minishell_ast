#include "../minishell.h"

static int	quoted_len(char *s)
{
	char	q;
	int		i;

	q = s[0];
	i = 1;
	while (s[i] && s[i] != q)
		i++;
	if (!s[i])
		return (-1);
	return (i + 1);
}

int	get_token_len(char *s)
{
	int	i;
	int	q;

	if (is_special(s[0]))
	{
		if ((s[0] == '<' && s[1] == '<') || (s[0] == '>' && s[1] == '>')
			|| (s[0] == '&' && s[1] == '&') || (s[0] == '|' && s[1] == '|'))
			return (2);
		return (1);
	}
	i = 0;
	while (s[i] && !is_whitespace(s[i]) && !is_special(s[i]))
	{
		if (s[i] == '\'' || s[i] == '"')
		{
			q = quoted_len(s + i);
			if (q < 0)
				return (-1);
			i += q;
		}
		else
			i++;
	}
	return (i);
}

static int	process_one_token(char **p, t_tok_list *list)
{
	int		len;
	char	*sub;

	len = get_token_len(*p);
	if (len <= 0)
		return (0);
	sub = extract_quoted(*p, len);
	if (!sub)
		return (0);
	if (!add_token(sub, assign_type(*p), list))
		return (free(sub), 0);
	*p += len;
	return (1);
}

t_token	*tokenize_line(char *line, int *err)
{
	t_tok_list list;
	char *p;

	*err = 0;
	list.head = NULL;
	list.tail = NULL;
	p = line;
	while (*p)
	{
		while (*p && is_whitespace(*p))
			p++;
		if (!*p)
			break ;
		if (!process_one_token(&p, &list))
		{
			free_token_list(list.head);
			*err = 2;
			ft_putendl_fd("minishell: syntax error: unclosed quote",
				STDERR_FILENO);
			return (NULL);
		}
	}
	return (list.head);
}