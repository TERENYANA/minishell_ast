
#include "../minishell.h"

void	append_redirect(t_node *cmd_node, t_redirect *r)
{
	t_redirect	*cur;

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
	return (r->target != NULL);
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
		return (free(r), NULL);
	return (r);
}

int	process_redir(t_node *cmd_node, t_token **cur, t_parse_info *info)
{
	t_redirect	*r;
	t_token		*op;

	op = *cur;
	if (!op->next || op->next->type != WORD)
	{
		if (info->error_code)
			*info->error_code = 2;
		return (0);
	}
	r = create_redirect(op, op->next, info);
	if (!r)
		return (0);
	append_redirect(cmd_node, r);
	*cur = op->next->next;
	return (1);
}