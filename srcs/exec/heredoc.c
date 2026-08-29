/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:13:50 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:13:50 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Interprets the heredoc child exit status and wires the pipe read end
 * back to the redirection when the child finished normally.
 */
static int	handle_heredoc_status(int status, int p[2], t_redirect *redir)
{
	setup_signal_handlers();
	if ((WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
		|| (WIFEXITED(status) && WEXITSTATUS(status) == 130))
		return (g_sig = SIGINT, close(p[0]), -1);
	redir->heredoc_fd = p[0];
	return (0);
}

/*
 * Creates a forked heredoc process, sends the input to a pipe, waits for it,
 * and stores the read end of the pipe in the associated redirection.
 */
static int	process_heredoc(t_hd *hd)
{
	int		p[2];
	pid_t	pid;
	int		status;

	if (pipe(p) == -1)
		return (-1);
	ignore_signals();
	pid = fork();
	if (pid == -1)
		return (close(p[0]), close(p[1]), -1);
	if (pid == 0)
		heredoc_child(p, hd);
	close(p[1]);
	waitpid(pid, &status, 0);
	return (handle_heredoc_status(status, p, hd->redir));
}

/*
 * Walks the AST and processes every heredoc found in command or subshell nodes.
 * Returns -1 if a heredoc is interrupted or fails during setup.
 */
int	process_all_heredocs(t_node *node, t_hd *hd)
{
	t_redirect	*r;

	if (!node)
		return (0);
	if (node->type == N_CMD || node->type == N_SUB)
	{
		r = node->redirect;
		while (r)
		{
			if (r->type == HEREDOC)
			{
				hd->redir = r;
				if (process_heredoc(hd) == -1)
					return (-1);
			}
			r = r->next;
		}
		if (node->type == N_CMD)
			return (0);
	}
	if (process_all_heredocs(node->left, hd) == -1)
		return (-1);
	return (process_all_heredocs(node->right, hd));
}

/*
 * Closes every heredoc pipe descriptor stored in the tree after execution so
 * the file descriptors are not left open for the next command.
 */
void	close_heredoc_fds(t_node *node)
{
	t_redirect	*r;

	if (!node)
		return ;
	if (node->type == N_CMD || node->type == N_SUB)
	{
		r = node->redirect;
		while (r)
		{
			if (r->type == HEREDOC && r->heredoc_fd >= 0)
			{
				close(r->heredoc_fd);
				r->heredoc_fd = -1;
			}
			r = r->next;
		}
	}
	close_heredoc_fds(node->left);
	close_heredoc_fds(node->right);
}
