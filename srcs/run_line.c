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

static int	err_or(int err, int fallback)
{
	if (err)
		return (err);
	return (fallback);
}

int	run_line(char *line, t_var **env, int status, int *abort)
{
	t_token	*tokens;
	t_node	*tree;
	int		err;

	err = 0;
	tokens = tokenize_line(line, &err);
	if (!tokens)
	{
		if (err)
			*abort = 1;
		return (err_or(err, status));
	}
	if (!syntax_ok(tokens, &err))
	{
		free_token_list(tokens);
		*abort = 1;
		return (2);
	}
	tree = parsing(tokens, *env, status, &err);
	free_token_list(tokens);
	if (process_all_heredocs(tree, tree, env, status, line) == -1)
	{
		ft_free_node(tree);
		*abort = 1;
		return (130);
	}
	status = run_tree(tree, env, status);
	close_heredoc_fds(tree);
	ft_free_node(tree);
	return (status);
}
