/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   syntax.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:13:13 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/20 17:31:34 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_logic(t_token_type t)
{
	return (t == PIPE || t == AND_IF || t == OR_IF);
}

static int	check_parens(t_token *t, int *error_code)
{
	int	depth;

	depth = 0;
	while (t)
	{
		if (t->type == LPAREN)
		{
			if (!t->next)
				return (syntax_err("newline", error_code));
			if (t->next->type == RPAREN || is_logic(t->next->type))
				return (syntax_err(t->next->value, error_code));
			depth++;
		}
		if (t->type == RPAREN)
		{
			depth--;
			if (depth < 0)
				return (syntax_err(")", error_code));
		}
		t = t->next;
	}
	if (depth > 0)
		return (syntax_err("newline", error_code));
	return (1);
}

static int	bad_neighbour(t_token *t)
{
	if (t->type == WORD && ft_strcmp(t->value, "&") == 0)
		return (1);
	if (t->type == WORD && t->next && t->next->type == LPAREN)
		return (1);
	if (t->type == RPAREN && t->next && t->next->type == WORD)
		return (1);
	return (0);
}

static int	check_operators(t_token *t, int *error_code)
{
	while (t)
	{
		if (bad_neighbour(t))
		{
			if (t->type == WORD && t->next && t->next->type == LPAREN)
				return (syntax_err("(", error_code));
			return (syntax_err(t->value, error_code));
		}
		if (is_logic(t->type))
		{
			if (!t->next)
				return (syntax_err("newline", error_code));
			if (is_logic(t->next->type) || t->next->type == RPAREN)
				return (syntax_err(t->next->value, error_code));
		}
		if (is_redir_tok(t->type) && (!t->next || t->next->type != WORD))
		{
			if (!t->next)
				return (syntax_err("newline", error_code));
			return (syntax_err(t->next->value, error_code));
		}
		t = t->next;
	}
	return (1);
}

int	syntax_ok(t_token *t, int *error_code)
{
	if (error_code)
		*error_code = 0;
	if (!t)
		return (1);
	if (is_logic(t->type))
		return (syntax_err(t->value, error_code));
	if (!check_operators(t, error_code))
		return (0);
	return (check_parens(t, error_code));
}
