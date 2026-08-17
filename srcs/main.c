#include "./minishell.h"

static int	err_or(int err, int fallback)
{
	if (err)
		return (err);
	return (fallback);
}

static int	run_line(char *line, t_var **env, int status)
{
	t_token	*tokens;
	t_node	*tree;
	int		err;

	err = 0;
	tokens = tokenize_line(line, &err);
	if (!tokens)
		return (err_or(err, status));
	if (!syntax_ok(tokens, &err))
	{
		free_token_list(tokens);
		return (2);
	}
	tree = parsing(tokens, *env, status, &err);
	free_token_list(tokens);
	if (!tree)
		return (err_or(err, 2));
	if (process_all_heredocs(tree, *env, status) == -1)
	{
		ft_free_node(tree);
		return (130);
	}
	status = run_tree(tree, env, status);
	ft_free_node(tree);
	return (status);
}

static int	post_readline_status(int prev)
{
	if (g_sig == SIGINT)
	{
		g_sig = 0;
		return (130);
	}
	return (prev);
}

static void	main_loop(t_var **env, int *status)
{
	char	*line;

	while (1)
	{
		setup_signal_handlers();
		line = readline("minishell$ ");
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
	rl_clear_history();
	ft_free_env(env);
	if (isatty(STDIN_FILENO))
		ft_putendl_fd("exit", STDOUT_FILENO);
	return (status);
}