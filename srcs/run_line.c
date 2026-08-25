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

#include "./minishell.h"

static int	handle_err(int err, int status, int *abort)
{
	if (err)
	{
		*abort = 1;
		return (err);
	}
	return (status);
}

static int	syntax_error(t_token *tokens, int *abort)
{
	free_token_list(tokens);
	*abort = 1;
	return (2);
}

static int	heredoc_error(t_node *tree, int *abort)
{
	ft_free_node(tree);
	*abort = 1;
	return (130);
}

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
