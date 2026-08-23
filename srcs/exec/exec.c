/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 15:42:59 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 15:43:01 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

void	expand_cmd_args(t_node *cmd_node, t_var *env, int status)
{
	char	**old_cmd;
	char	*expanded;
	int		i;

	if (!cmd_node || !cmd_node->cmd)
		return ;
	old_cmd = cmd_node->cmd;
	cmd_node->cmd = NULL;
	i = 0;
	while (old_cmd[i])
	{
		expanded = ft_expand(old_cmd[i], env, status);
		if (expanded && expanded[0] == '\0' && !has_quotes(old_cmd[i]))
			free(expanded);
		else if (expanded && has_unquoted_star(old_cmd[i]))
			add_wildcard_args(cmd_node, expanded);
		else if (expanded)
			add_arg(cmd_node, expanded);
		i++;
	}
	ft_free_tab(old_cmd);
}

static int	run_builtin_in_parent(t_node *node, t_var **env, int last_status)
{
	int	saved_in;
	int	saved_out;
	int	ret;

	saved_in = dup(STDIN_FILENO);
	saved_out = dup(STDOUT_FILENO);
	if (saved_in < 0 || saved_out < 0)
	{
		if (saved_in >= 0)
			close(saved_in);
		if (saved_out >= 0)
			close(saved_out);
		return (1);
	}
	expand_cmd_args(node, *env, last_status);
	if (apply_redirections(node, *env, last_status) != 0)
		ret = 1;
	else
	{
		if (!node->cmd || !node->cmd[0])
			ret = 0;
		else
			ret = dispatch_builtin(node, env, last_status);
	}
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
	return (ret);
}

static int	is_parent_builtin_root(t_node *root)
{
	return (root->type == N_CMD
		&& root->cmd && root->cmd[0]
		&& is_builtin(root->cmd[0]));
}

static int	need_right(t_node_type t, int status)
{
	if (t == N_AND)
		return (status == 0);
	return (status != 0);
}

int	run_tree(t_node *root, t_var **env, int last_status)
{
	int	status;

	if (root->type == N_AND || root->type == N_OR)
	{
		status = run_tree(root->left, env, last_status);
		if (need_right(root->type, status))
			status = run_tree(root->right, env, status);
		return (status);
	}
	if (is_parent_builtin_root(root))
		return (run_builtin_in_parent(root, env, last_status));
	return (fork_and_run(root, env, last_status));
}
