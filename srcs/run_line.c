/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_line.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 17:46:49 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/20 14:36:25 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Turns an error code from an earlier stage (tokenizing or parsing)
** into this line's final status. If err is non-zero, something
** fatal happened before a tree even exists to run, so *abort is
** raised (main_loop will stop reading further lines when in
** non-interactive/piped mode -- see main.c) and err becomes the
** exit status. If err is 0 (nothing actually went wrong -- e.g.
** tokens is NULL simply because the line was empty/whitespace-only),
** the previous status is returned unchanged, exactly like bash
** leaves $? untouched after an empty line.
*/
static int	handle_err(int err, int status, int *abort)
{
	if (err)
	{
		*abort = 1;
		return (err);
	}
	return (status);
}

/*
** Called when syntax_ok() rejects the token list (e.g. "| |",
** a line starting with "&&"). The tokens are freed here since
** parsing() was never reached and never took ownership of them.
** *abort is raised and 2 is returned -- bash's own conventional
** exit code for a shell syntax error.
*/
static int	syntax_error(t_token *tokens, int *abort)
{
	free_token_list(tokens);
	*abort = 1;
	return (2);
}

/*
** Called when a heredoc read is interrupted by Ctrl+C (SIGINT)
** before its delimiter line was reached (see process_all_heredocs
** in heredoc.c). The half-built tree is freed since command
** execution was never started, and 130 (128+SIGINT) is returned,
** matching bash's convention for any command killed by a signal.
*/
static int	heredoc_error(t_node *tree, int *abort)
{
	ft_free_node(tree);
	*abort = 1;
	return (130);
}

/*
** Runs one full logical line end-to-end, in five stages, each
** owning and freeing its own intermediate structure so a failure
** at any point never leaks the previous stage's allocation:
**   1. tokenize_line       : raw text -> t_token list
**   2. syntax_ok            : rejects malformed token sequences
**      before any tree is built
**   3. parsing               : t_token list -> command tree (t_node);
**      tokens are freed right after, since the tree owns its own
**      copies of everything it needs from them
**   4. process_all_heredocs : reads every heredoc body up front,
**      before any command actually runs -- this matches bash,
**      which captures heredoc content at parse time even for
**      commands that end up never executing (e.g. the losing side
**      of a failed && chain)
**   5. run_tree               : executes the tree (fork/execve/
**      builtins), producing the line's final exit status
** t_hd bundles everything process_all_heredocs needs (the tree
** root for recursion, env/status for $VAR expansion inside heredoc
** bodies, and the original line for reference) into one struct so
** it can be passed around without a long parameter list.
*/
int	run_line(char *line, t_var **env, int status, int *abort)
{
	t_token	*tokens;
	t_node	*tree;
	t_hd	hd;
	int		err;

	err = 0;
	tokens = tokenize_line(line, &err);
	if (!tokens)
		return (handle_err(err, status, abort));
	if (!syntax_ok(tokens, &err))
		return (syntax_error(tokens, abort));
	tree = parsing(tokens, *env, status, &err);
	free_token_list(tokens);
	if (!tree)
		return (handle_err(err, status, abort));
	hd.root = tree;
	hd.env = env;
	hd.status = status;
	hd.line = line;
	if (process_all_heredocs(tree, &hd) == -1)
		return (heredoc_error(tree, abort));
	status = run_tree(tree, env, status);
	close_heredoc_fds(tree);
	ft_free_node(tree);
	return (status);
}