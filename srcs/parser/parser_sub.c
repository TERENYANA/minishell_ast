/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_sub.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:12:58 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 14:11:48 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Handles '(' ... ')' blocks. parse_primary is what parse_pipeline
** actually calls for each side of a pipe: if the next token isn't
** '(', it's just an ordinary command; if it is, everything up to
** the matching ')' is parsed as a full nested list (parse_list, so
** &&/|| work inside parentheses too) and the resulting subtree is
** wrapped in a single N_SUB node by wrap_sub -- which also picks up
** any redirects trailing the ')', since those apply to the whole
** block, not to one command inside it.
*/

static t_node	*wrap_sub(t_node *inner, t_token **cur, t_parse_info *info)
{
	t_node	*sub;

	sub = new_op_node(N_SUB, inner, NULL);
	if (!sub)
		return (NULL);
	while (*cur && is_redir_tok((*cur)->type))
	{
		if (!process_redir(sub, cur, info))
			return (ft_free_node(sub), NULL);
	}
	return (sub);
}

t_node	*parse_primary(t_token **cur, t_parse_info *info)
{
	t_node	*inner;

	if (!*cur)
		return (NULL);
	if ((*cur)->type != LPAREN)
		return (parse_command(cur, info));
	*cur = (*cur)->next;
	inner = parse_list(cur, info);
	if (!inner)
		return (NULL);
	if (!*cur || (*cur)->type != RPAREN)
	{
		ft_free_node(inner);
		if (*cur)
			syntax_err((*cur)->value, info->error_code);
		else
			syntax_err("newline", info->error_code);
		return (NULL);
	}
	*cur = (*cur)->next;
	return (wrap_sub(inner, cur, info));
}
