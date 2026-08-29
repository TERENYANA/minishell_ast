/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:14:04 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:14:04 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Expands each argument of a command node using shell expansion rules,
 * handling empty results, wildcard expansion, and storing the final arguments.
 */
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

/*
 * Executes a parent-side builtin command after expansion and redirection.
 * It returns the builtin result while preserving the shell state.
 */
static int	execute_parent_builtin(t_node *node, t_var **env, int last_status)
{
	expand_cmd_args(node, *env, last_status);
	if (apply_redirections(node, *env, last_status) != 0)
		return (1);
	if (!node->cmd || !node->cmd[0])
		return (0);
	return (dispatch_builtin(node, env, last_status));
}

/*
 * Runs a builtin in the parent shell while temporarily saving stdin/stdout,
 * so redirections are contained to the builtin call and then restored.
 */
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
	ret = execute_parent_builtin(node, env, last_status);
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
	return (ret);
}

/*
 * Checks whether the current root node is a command that should be executed
 * as a builtin directly in the parent process.
 */
static int	is_parent_builtin_root(t_node *root)
{
	return (root->type == N_CMD
		&& root->cmd && root->cmd[0]
		&& (is_builtin(root->cmd[0]) || is_env_builtin(root->cmd)));
}

/*
 * Recursively executes the AST: logical AND/OR chains are evaluated left-to-right,
 * parent builtins run directly in the shell, and everything else is executed in a forked child.
 */
int	run_tree(t_node *root, t_var **env, int last_status)
{
	int	status;

	if (root->type == N_AND || root->type == N_OR)
	{
		status = run_tree(root->left, env, last_status);
		if ((root->type == N_AND && status == 0)
			|| (root->type == N_OR && status != 0))
			status = run_tree(root->right, env, status);
		return (status);
	}
	if (is_parent_builtin_root(root))
		return (run_builtin_in_parent(root, env, last_status));
	return (fork_and_run(root, env, last_status));
}
