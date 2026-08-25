/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 17:47:16 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/20 14:36:17 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./minishell.h"

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
 ** Lecture d'une ligne selon le contexte :
 **   - terminal (tty)  -> readline, avec prompt, historique, edition
 **   - pipe / fichier  -> get_next_line, sans prompt
 ** get_next_line laisse le '\n' final : on le retire pour que la commande
 ** soit reconnue (sinon "env\n" != "env").
 */
static char	*read_input(void)
{
	char	*line;
	size_t	len;

	if (isatty(STDIN_FILENO))
		return (readline("minishell$ "));
	line = get_next_line(STDIN_FILENO);
	if (!line)
		return (NULL);
	len = ft_strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len - 1] = '\0';
	return (line);
}

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
	rl_clear_history();
	ft_free_env(env);
	if (isatty(STDIN_FILENO))
		ft_putendl_fd("exit", STDOUT_FILENO);
	return (status);
}
