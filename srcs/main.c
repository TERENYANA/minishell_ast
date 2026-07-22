#include "minishell.h"

/*
** MacOS compatibility layer: libedit hook for rl_replace_line.
** Manually declaring the prototype to avoid implicit function declaration errors.
*/
void	rl_replace_line(const char *text, int clear_undo);

/*
** Temporary executor placeholder (Phase 1).
** Tokenizes the line, prints tokens for debugging, frees them.
** Returns err (2) on syntax error, otherwise passes status forward.
*/
static int	run_line(char *line, t_var *env, int status)
{
	t_token	*tokens;
	t_node	*tree;
	int		err;

	tokens = tokenize_line(line, &err);
	if (!tokens)
		return (status);
	if (!syntax_ok(tokens, &err))
		return (free_token_list(tokens), err);
	tree = parsing(tokens, env, status, &err);
	free_token_list(tokens);
	if (!tree)
		return (err ? err : status);
	print_tree(tree, 0);            /* test */
	ft_free_node(tree);
	return (status);
}

/*
** Evaluates shell state immediately after readline returns.
** If a SIGINT (Ctrl+C) was caught, resets the global flag and returns 130
** (Standard Unix exit status for processes terminated by SIGINT: 128 + 2).
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
		line = readline("minishell$ ");
		if (!line)
			break ;
		status = post_readline_status(status);
		if (*line)
			add_history(line);
		status = run_line(line, env, status);
		free(line);
	}
	ft_free_env(env);
	ft_putendl_fd("exit", STDERR_FILENO);
	rl_clear_history();
	return (status);
}