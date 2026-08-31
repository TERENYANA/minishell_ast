/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   read_input.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 13:42:00 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/31 20:27:19 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 ** True if the line ends with an unclosed quote (single or double),
 ** taking backslash-escaping inside double quotes into account
 ** (same logic as quoted_len in lexer.c). Used to detect that a
 ** continuation line (PS2) must be read, exactly like bash does.
 */
static int	line_has_open_quote(const char *s)
{
	char	q;
	int		i;

	q = 0;
	i = 0;
	while (s[i])
	{
		if (q)
		{
			if (q == '"' && s[i] == '\\' && s[i + 1])
				i++;
			else if (s[i] == q)
				q = 0;
		}
		else if (s[i] == '\'' || s[i] == '"')
			q = s[i];
		i++;
	}
	return (q != 0);
}

/*
 ** Joins the current line and the continuation line with a '\n'
 ** in between, rebuilding the logical command the same way bash
 ** does after an unclosed-quote continuation.
 ** Frees a and b in every case (success or failure).
 */
static char	*join_line(char *a, char *b)
{
	char	*tmp;
	char	*res;

	if (!b)
		return (free(a), NULL);
	tmp = ft_strjoin(a, "\n");
	free(a);
	if (!tmp)
		return (free(b), NULL);
	res = ft_strjoin(tmp, b);
	free(tmp);
	free(b);
	return (res);
}

/*
 ** Reads one continuation line (after an unclosed quote).
 ** Same tty/pipe logic as read_input: "> " prompt when interactive,
 ** get_next_line with no prompt otherwise. Returns NULL if EOF is
 ** reached before the continuation is provided (the line then stays
 ** with an unclosed quote, and run_line will report the syntax error).
 */
static char	*read_continuation(void)
{
	char	*line;
	size_t	len;

	if (isatty(STDIN_FILENO))
		return (readline("> "));
	line = get_next_line(STDIN_FILENO);
	if (!line)
		return (NULL);
	len = ft_strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len - 1] = '\0';
	return (line);
}

/*
 ** Reads one line depending on context:
 **   - terminal (tty)  -> readline, with prompt, history, editing
 **   - pipe / file     -> get_next_line, no prompt
 ** get_next_line keeps the trailing '\n': we strip it so the command
 ** is recognized correctly (otherwise "env\n" != "env").
 ** If the resulting line ends with an unclosed quote, continuation
 ** lines are read (bash's PS2 behavior) and appended with '\n' until
 ** the quotes are balanced or EOF is reached.
 */
char	*read_input(void)
{
	char	*line;
	char	*cont;
	size_t	len;

	if (isatty(STDIN_FILENO))
		line = readline("minishell$ ");
	else
	{
		line = get_next_line(STDIN_FILENO);
		if (line)
		{
			len = ft_strlen(line);
			if (len > 0 && line[len - 1] == '\n')
				line[len - 1] = '\0';
		}
	}
	while (line && line_has_open_quote(line))
	{
		cont = read_continuation();
		if (!cont)
			break ;
		line = join_line(line, cont);
	}
	return (line);
}
