#include "minishell.h"

static int sub_redirs(t_node *sub, t_token **cur, t_parse_info *info)
{
    while (*cur && is_redir_tok((*cur)->type))
    {
        if (!process_redir(sub, cur, info))
            return (0);
    }
    return (1);
}

/* primary := '(' list ')' redir*  |  command */
t_node *parse_primary(t_token **cur, t_parse_info *info)
{
    t_node *inner;
    t_node *sub;

    if (!*cur || (*cur)->type != LPAREN)
        return (parse_command(cur, info));
    *cur = (*cur)->next;
    inner = parse_list(cur, info); /* ЛИФТ: снова верхний этаж */
    if (!inner)
        return (NULL);
    if (!*cur || (*cur)->type != RPAREN)
        return (ft_free_node(inner), syntax_err(")", info->error_code), NULL);
    *cur = (*cur)->next;
    sub = new_op_node(N_SUB, inner, NULL);
    if (!sub)
        return (NULL);
    if (!sub_redirs(sub, cur, info))
        return (ft_free_node(sub), NULL);
    return (sub);
}