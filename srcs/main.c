/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 17:47:16 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 17:48:52 by yyuskiv          ###   ########.fr       */
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

static void	main_loop(t_var **env, int *status)
{
	char	*line;

	while (1)
	{
		setup_signal_handlers();
		line = read_input();
		if (!line)
			break ;
		*status = post_readline_status(*status);
		if (*line && isatty(STDIN_FILENO))
			add_history(line);
		*status = run_line(line, env, *status);
		free(line);
	}
}

int	main(int argc, char **argv, char **envp)
{
	t_var	*env;
	int		status;

	(void)argc;
	(void)argv;
	env = create_env(envp);
	status = 0;
	main_loop(&env, &status);
	get_next_line(-1);
	rl_clear_history();
	ft_free_env(env);
	if (isatty(STDIN_FILENO))
		ft_putendl_fd("exit", STDOUT_FILENO);
	return (status);
}
