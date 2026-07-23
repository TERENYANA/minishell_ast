#include "../minishell.h"

/* Erreur de syntaxe : code 2, liberation du noeud partiel, retour NULL. */
t_node *syntax_err_node(t_node *node, t_parse_info *info)
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

/* Mot vide sans guillemets : on le jette (comportement bash). */
int add_word(t_node *node, t_token *tok, t_parse_info *info)
{
    char *expanded;

    expanded = ft_expand(tok->value, info->env, info->status);
    if (!expanded)
        return (0);
    if (expanded[0] == '\0' && !has_quotes(tok->value))
        return (free(expanded), 1);
    return (add_arg(node, expanded));
}

int is_cmd_end(t_token *t)
{
    if (!t)
        return (1);
    return (t->type == PIPE || t->type == AND_IF || t->type == OR_IF || t->type == RPAREN);
}
