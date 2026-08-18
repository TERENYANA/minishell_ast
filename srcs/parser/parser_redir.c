/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redir.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:12:34 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 16:12:36 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

void	append_redirect(t_node *cmd_node, t_redirect *r)
{
	t_redirect	*cur;

	if (!cmd_node || !r)
		return ;
	if (!cmd_node->redirect)
	{
		cmd_node->redirect = r;
		return ;
	}
	cur = cmd_node->redirect;
	while (cur->next)
		cur = cur->next;
	cur->next = r;
}

static int	setup_heredoc(t_redirect *r, t_token *target, t_parse_info *info)
{
	r->expand_heredoc = !has_quotes(target->value);
	r->target = ft_expand(target->value, info->env, info->status);
	r->heredoc_fd = -1;
	return (r->target != NULL);
}

static int	setup_file_target(t_redirect *r, t_token *target,
		t_parse_info *info)
{
	r->expand_heredoc = 0;
	r->heredoc_fd = -1;
	r->target = ft_expand(target->value, info->env, info->status);
	if (!r->target)
		return (0);
	if (r->target[0] == '\0' && !has_quotes(target->value))
	{
		ft_putstr_fd("minishell: ", 2);
		ft_putstr_fd(target->value, 2);
		ft_putendl_fd(": ambiguous redirect", 2);
		return (0);
	}
	return (1);
}

t_redirect	*create_redirect(t_token *op, t_token *target, t_parse_info *info)
{
	t_redirect	*r;
	int			ok;

	r = ft_calloc(1, sizeof(t_redirect));
	if (!r)
		return (NULL);
	r->type = op->type;
	if (op->type == HEREDOC)
		ok = setup_heredoc(r, target, info);
	else
		ok = setup_file_target(r, target, info);
	if (!ok)
		return (free(r->target), free(r), NULL);
	return (r);
}

int	process_redir(t_node *cmd_node, t_token **cur, t_parse_info *info)
{
	t_redirect	*r;
	t_token		*op;

	op = *cur;
	if (!op || !op->next || op->next->type != WORD)
	{
		if (info->error_code)
			*info->error_code = 2;
		if (*cur)
			*cur = (*cur)->next;
		return (0);
	}
	r = create_redirect(op, op->next, info);
	if (!r)
	{
		if (info->error_code)
			*info->error_code = 1;
		*cur = op->next->next;
		return (0);
	}
	append_redirect(cmd_node, r);
	*cur = op->next->next;
	return (1);
}
