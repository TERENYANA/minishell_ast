/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   syntax_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:13:27 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/28 14:11:49 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Small shared helpers used by both the pre-parse validator
** (syntax.c) and the parser itself (parser*.c): classifying
** redirect-type tokens, and printing bash's conventional
** "syntax error near unexpected token" message. Kept separate from
** syntax.c so parser code can call is_redir_tok/syntax_err without
** depending on the validation logic itself.
*/

int	is_redir_tok(t_token_type t)
{
	return (t == REDIR_IN || t == REDIR_OUT
		|| t == REDIR_APPEND || t == HEREDOC);
}

int	syntax_err(const char *tok, int *error_code)
{
	ft_putstr_fd("minishell: syntax error near unexpected token `", 2);
	if (tok)
		ft_putstr_fd((char *)tok, 2);
	else
		ft_putstr_fd("newline", 2);
	ft_putendl_fd("'", 2);
	if (error_code)
		*error_code = 2;
	return (0);
}
