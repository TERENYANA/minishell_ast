#include "../minishell.h"

static int	process_heredoc(t_redirect *redir, t_var *env, int status_val)
{
	int		p[2];
	pid_t	pid;
	int		status;
	t_hd	hd;

	hd.redir = redir;
	hd.env = env;
	hd.status = status_val;
	if (pipe(p) == -1)
		return (-1);
	ignore_signals();
	pid = fork();
	if (pid == -1)
		return (close(p[0]), close(p[1]), -1);
	if (pid == 0)
		heredoc_child(p, &hd);
	close(p[1]);
	waitpid(pid, &status, 0);
	setup_signal_handlers();
	if ((WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		|| (WIFEXITED(status) && WEXITSTATUS(status) == 130))
		return (g_sig = SIGINT, close(p[0]), -1);
	redir->heredoc_fd = p[0];
	return (0);
}

int	process_all_heredocs(t_node *node, t_var *env, int status)
{
	t_redirect	*r;

	if (!node)
		return (0);
	if (node->type == N_CMD)
	{
		r = node->redirect;
		while (r)
		{
			if (r->type == HEREDOC && process_heredoc(r, env, status) == -1)
				return (-1);
			r = r->next;
		}
		return (0);
	}
	if (process_all_heredocs(node->left, env, status) == -1)
		return (-1);
	return (process_all_heredocs(node->right, env, status));
}