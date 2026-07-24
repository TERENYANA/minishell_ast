#include "minishell.h"

void	rl_replace_line(const char *text, int clear_undo);

static void	trim_newline(char *line)
{
	size_t	len;

	if (!line)
		return ;
	len = ft_strlen(line);
	if (len > 0 && line[len - 1] == '\n')
		line[len - 1] = '\0';
}

static int	run_line(char *line, t_var **env, int status)
{
	t_token	*tokens;
	t_node	*tree;
	int		err;

	err = 0;
	tokens = tokenize_line(line, &err);
	if (!tokens)
		return (err ? err : status);
	if (!syntax_ok(tokens, &err))
	{
		free_token_list(tokens);
		return (2);
	}
	tree = parsing(tokens, *env, status, &err);
	free_token_list(tokens);
	if (!tree)
		return (err ? err : 2);
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

int	main(int argc, char **argv, char **envp)
{
	t_var	*env;
	char	*line;
	int		status;

	(void)argc;
	(void)argv;
	env = create_env(envp);
	status = 0;
	while (1)
	{
		setup_signal_handlers();
		if (isatty(STDIN_FILENO))
			line = readline("minishell$ ");
		else
			line = get_next_line(STDIN_FILENO);
		if (!line)
			break ;
		trim_newline(line);
		status = post_readline_status(status);
		if (*line && isatty(STDIN_FILENO))
			add_history(line);
		status = run_line(line, &env, status);
		free(line);
	}
	get_next_line(-1); // <-- КРИТИЧНО: полностью очищает статический stash при выходе из shell
	rl_clear_history();
	ft_free_env(env);
	if (isatty(STDIN_FILENO))
		ft_putendl_fd("exit", STDERR_FILENO);
	return (status);
}