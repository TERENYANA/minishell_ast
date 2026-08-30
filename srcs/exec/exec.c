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
** "exit" is special-cased: instead of calling ft_exit() here (which
** would exit() immediately and skip the fd cleanup in
** run_builtin_in_parent below), it signals the caller with -2 so
** fds get restored/closed first, and ft_exit() is called only after.
*/
static int	execute_parent_builtin(t_node *cur, t_var **env, int last_status)
{
	expand_cmd_args(cur, *env, last_status);
	if (apply_redirections(cur, *env, last_status) != 0)
		return (1);
	if (!cur->cmd || !cur->cmd[0])
		return (0);
	if (ft_strcmp(cur->cmd[0], "exit") == 0)
		return (-2);
	return (dispatch_builtin(cur, env, last_status));
}

/*
** root is passed through separately from cur so that if this
** builtin is "exit", ft_exit()/cleanup_and_exit() frees the WHOLE
** tree (root), not just the subnode currently being executed --
** otherwise, when exit occurs inside a && / || chain, only that
** leaf node gets freed and the rest of the tree leaks.
*/
static int	run_builtin_in_parent(t_node *root, t_node *cur, t_var **env,
		int last_status)
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
	ret = execute_parent_builtin(cur, env, last_status);
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
	if (ret == -2)
		ft_exit(root, cur, env, last_status);
	return (ret);
}

static int	is_parent_builtin_root(t_node *cur)
{
	return (cur->type == N_CMD
		&& cur->cmd && cur->cmd[0]
		&& (is_builtin(cur->cmd[0]) || is_env_builtin(cur->cmd)));
}

/*
** root stays fixed at the top of the tree through every recursive
** call; cur is the node actually being evaluated right now. This
** split matters because fork_and_run/run_builtin_in_parent/
** cleanup_and_exit all need the REAL root to free the whole tree,
** even when we're several && / || levels deep into it.
*/
static int	run_node(t_node *root, t_node *cur, t_var **env, int last_status)
{
	int	status;

	if (cur->type == N_AND || cur->type == N_OR)
	{
		status = run_node(root, cur->left, env, last_status);
		if ((cur->type == N_AND && status == 0)
			|| (cur->type == N_OR && status != 0))
			status = run_node(root, cur->right, env, status);
		return (status);
	}
	if (is_parent_builtin_root(cur))
		return (run_builtin_in_parent(root, cur, env, last_status));
	return (fork_and_run(root, cur, env, last_status));
}

int	run_tree(t_node *root, t_var **env, int last_status)
{
	return (run_node(root, root, env, last_status));
}