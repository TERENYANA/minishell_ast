/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run_line.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 17:46:49 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 17:46:56 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./minishell.h"

static int	err_or(int err, int fallback)
{
	if (err)
		return (err);
	return (fallback);
}

int	run_line(char *line, t_var **env, int status)
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
