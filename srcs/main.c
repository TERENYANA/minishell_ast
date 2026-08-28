/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:12:21 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:12:21 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	post_readline_status(int prev)
{
	if (g_sig == SIGINT)
	{
		g_sig = 0;
		return (130);
	}
	return (prev);
}

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
static char	*read_input(void)
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

/*
 ** Initializes the environment variables managed by the shell itself
 ** (as opposed to those inherited as-is from envp): SHLVL incremented,
 ** OLDPWD created empty if absent, and '_' set the way a real exec
 ** would leave it.
 */
static void	init_env_vars(t_var **env)
{
	char	*shlvl_str;
	int		shlvl;

	shlvl_str = get_env_value("SHLVL", *env);
	if (shlvl_str)
		shlvl = ft_atoi(shlvl_str) + 1;
	else
		shlvl = 1;
	shlvl_str = ft_itoa(shlvl);
	env_set(env, "SHLVL", shlvl_str);
	if (!find_var(*env, "OLDPWD"))
		env_set(env, "OLDPWD", NULL);
	env_set(env, "_", ft_strdup("/usr/bin/env"));
}

/*
 ** Main REPL loop: reads a line (with continuation if needed), updates
 ** status after a SIGINT received during readline, runs the line, and
 ** stops on EOF or on a fatal error in non-interactive mode (pipe/file).
 */
static void	main_loop(t_var **env, int *status)
{
	char	*line;
	int		abort;

	while (1)
	{
		setup_signal_handlers();
		line = read_input();
		if (!line)
			break ;
		*status = post_readline_status(*status);
		if (*line && isatty(STDIN_FILENO))
			add_history(line);
		abort = 0;
		*status = run_line(line, env, *status, &abort);
		free(line);
		if (abort && !isatty(STDIN_FILENO))
			break ;
	}
}

/*
 ** Entry point: builds the environment list from envp, runs the main
 ** loop, then cleans up properly (history, env, get_next_line) before
 ** returning the last status.
 */
int	main(int argc, char **argv, char **envp)
{
	t_var	*env;
	int		status;

	(void)argc;
	(void)argv;
	env = create_env(envp);
	init_env_vars(&env);
	status = 0;
	main_loop(&env, &status);
	get_next_line(-1);
	clear_history();
	ft_free_env(env);
	if (isatty(STDIN_FILENO))
		ft_putendl_fd("exit", STDOUT_FILENO);
	return (status);
}