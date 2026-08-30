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

/*
** After readline() returns, this checks whether the user pressed
** Ctrl+C while typing (readline itself catches SIGINT and returns
** an empty line, but our signal handler sets g_sig=SIGINT to signal
** this). If so, the shell's exit status must reflect that a signal
** interrupted the input line (128+SIGINT=130).
*/
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
 ** Initializes the environment variables managed by the shell itself
 ** (as opposed to those inherited as-is from envp): SHLVL incremented,
 ** OLDPWD created empty if absent. '_' is left as inherited from envp
 ** and is only updated
 ** later, when this shell itself execve's an external command.
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
}

/*
 ** Main REPL loop: reads a line (with continuation if needed), updates
 ** status after a SIGINT received during readline, runs the line, and
 ** stops on EOF or on a fatal error in non-interactive mode (pipe/file).
 line = "echo \"Hello\""
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
