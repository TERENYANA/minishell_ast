#include "../minishell.h"

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
	if (apply_redirections(node) != 0)
		ret = 1;
	else if (!node->cmd || !node->cmd[0])
		ret = 0;
	else
		ret = dispatch_builtin(node, env, last_status);
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
	return (fork_and_run(root, env));
}