#include "../minishell.h"

/*
** GRAMMAIRE
**   list     := pipeline (('&&' | '||') pipeline)*   faibles — pres de la racine
**   pipeline := primary  ('|' primary)*              fort    — plus profond
**   primary  := '(' list ')' redir*  |  command      ascenseur / atome
**   command  := (WORD | redirection)+                feuilles
*/

t_node *parse_command(t_token **cur, t_parse_info *info)
{
    t_node *node;

    node = new_cmd_node();
    if (!node)
        return (NULL);
    while (!is_cmd_end(*cur))
    {
        if ((*cur)->type == WORD)
        {
            if (!add_word(node, *cur, info))
                return (ft_free_node(node), NULL);
            *cur = (*cur)->next;
        }
        else if (!process_redir(node, cur, info))
            return (ft_free_node(node), NULL);
    }
    if (!node->cmd && !node->redirect)
        return (syntax_err_node(node, info));
    return (node);
}

t_node *parse_pipeline(t_token **cur, t_parse_info *info)
{
    t_node *left;
    t_node *right;

    left = parse_primary(cur, info);
    if (!left)
        return (NULL);
    while (*cur && (*cur)->type == PIPE)
    {
        *cur = (*cur)->next;
        right = parse_primary(cur, info);
        if (!right)
            return (ft_free_node(left), NULL);
        left = new_op_node(N_PIPE, left, right);
        if (!left)
            return (NULL);
    }
    return (left);
}

t_node *parse_list(t_token **cur, t_parse_info *info)
{
    t_node *left;
    t_node *right;
    t_node_type op;

    left = parse_pipeline(cur, info);
    if (!left)
        return (NULL);
    while (*cur && ((*cur)->type == AND_IF || (*cur)->type == OR_IF))
    {
        op = N_OR;
        if ((*cur)->type == AND_IF)
            op = N_AND;
        *cur = (*cur)->next;
        right = parse_pipeline(cur, info);
        if (!right)
            return (ft_free_node(left), NULL);
        left = new_op_node(op, left, right);
        if (!left)
            return (NULL);
    }
    return (left);
}

t_node *parsing(t_token *head, t_var *env, int status, int *error_code)
{
    t_parse_info info;
    t_token *cur;

    info.env = env;
    info.status = status;
    info.error_code = error_code;
    if (error_code)
        *error_code = 0;
    cur = head;
    return (parse_list(&cur, &info));
}
