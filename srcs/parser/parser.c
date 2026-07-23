#include "../minishell.h"

/*
** GRAMMAIRE
**   list     := pipeline (('&&' | '||') pipeline)*   faibles — pres de la racine
**   pipeline := primary  ('|' primary)*              fort    — plus profond
**   primary  := '(' list ')' redir*  |  command      ascenseur / atome
**   command  := (WORD | redirection)+                feuilles
*/

// CONSTRUCTEURS

t_node *new_cmd_node(void)
{
    t_node *n;

    n = ft_calloc(1, sizeof(t_node));
    if (!n)
        return (NULL);
    n->type = N_CMD;
    return (n);
}

t_node *new_op_node(t_node_type type, t_node *left, t_node *right)
{
    t_node *n;

    n = ft_calloc(1, sizeof(t_node));
    if (!n)
        return (ft_free_node(left), ft_free_node(right), NULL);
    n->type = type;
    n->left = left;
    n->right = right;
    return (n);
}

// HELPERS

/* Erreur de syntaxe : code 2, liberation du noeud partiel, retour NULL. */
static t_node *syntax_err_node(t_node *node, t_parse_info *info)
{
    if (info->error_code)
        *info->error_code = 2;
    ft_free_node(node);
    return (NULL);
}

int add_arg(t_node *node, char *value)
{
    char **arr;
    int i;

    i = 0;
    while (node->cmd && node->cmd[i])
        i++;
    arr = malloc(sizeof(char *) * (i + 2));
    if (!arr)
        return (free(value), 0);
    i = 0;
    while (node->cmd && node->cmd[i])
    {
        arr[i] = node->cmd[i];
        i++;
    }
    arr[i] = value;
    arr[i + 1] = NULL;
    free(node->cmd);
    node->cmd = arr;
    return (1);
}



int is_cmd_end(t_token *t)
{
    if (!t)
        return (1);
    return (t->type == PIPE || t->type == AND_IF || t->type == OR_IF || t->type == RPAREN);
}
static int  add_word(t_node *node, t_token *tok, t_parse_info *info)
{
    char    *expanded;

    expanded = ft_expand(tok->value, info->env, info->status);
    if (!expanded)
        return (0);
    if (expanded[0] == '\0' && !has_quotes(tok->value))
        return (free(expanded), 1);

    // 🐛 DEBUG PRINT
    printf("[DEBUG] tok->value = '%s', has_star = %d\n", 
           tok->value, has_unquoted_star(tok->value));

    if (has_unquoted_star(tok->value))            /* ← СЫРОЙ токен! */
        return (add_wildcard_args(node, expanded));
    return (add_arg(node, expanded));
}
// GRAMMAIRE : command (feuilles)

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

// GRAMMAIRE : primary (atome / ascenseur)

/* Redirections placees APRES la parenthese fermante : (echo hi) > out */
static int read_sub_redirs(t_node *sub, t_token **cur, t_parse_info *info)
{
    while (*cur && is_redir_tok((*cur)->type))
    {
        if (!process_redir(sub, cur, info))
            return (0);
    }
    return (1);
}

t_node *parse_primary(t_token **cur, t_parse_info *info)
{
    t_node *inner;
    t_node *sub;

    if (!*cur || (*cur)->type != LPAREN)
        return (parse_command(cur, info));
    *cur = (*cur)->next;
    inner = parse_list(cur, info);
    if (!inner)
        return (NULL);
    if (!*cur || (*cur)->type != RPAREN)
        return (syntax_err_node(inner, info));
    *cur = (*cur)->next;
    sub = new_op_node(N_SUB, inner, NULL);
    if (!sub)
        return (NULL);
    if (!read_sub_redirs(sub, cur, info))
        return (ft_free_node(sub), NULL);
    return (sub);
}
// GRAMMAIRE : pipeline et list

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

/* Meme accumulation vers la gauche que pipeline, avec deux operateurs. */
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

// POINT D'ENTREE

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