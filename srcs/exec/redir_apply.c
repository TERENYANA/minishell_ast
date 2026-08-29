/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_apply.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:13:41 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:13:41 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Opens the file or pipe associated with a redirection based on its type.
 */
static int	open_redir(t_redirect *r)
{
	if (!r)
		return (-1);
	if (r->type == HEREDOC)
		return (r->heredoc_fd);
	if (r->type == REDIR_IN)
		return (open(r->target, O_RDONLY));
	if (r->type == REDIR_APPEND)
		return (open(r->target, O_WRONLY | O_CREAT | O_APPEND, 0644));
	return (open(r->target, O_WRONLY | O_CREAT | O_TRUNC, 0644));
}

/*
 * Prints the standard shell error message for a redirection failure.
 */
static int	perror_redir(const char *target)
{
	ft_putstr_fd("minishell: ", 2);
	if (target)
		perror((char *)target);
	else
		ft_putendl_fd("ambiguous redirect", 2);
	return (1);
}

/*
 * Reports an ambiguous redirect after expansion and frees temporary matches.
 */
static int	ambiguous_redir_error(const char *target, char *exp, char **m)
{
	ft_putstr_fd("minishell: ", 2);
	ft_putstr_fd((char *)target, 2);
	ft_putendl_fd(": ambiguous redirect", 2);
	if (exp)
		free(exp);
	if (m)
		ft_free_tab(m);
	return (1);
}

/*
 * Expands a redirection target, resolves wildcard matches if needed,
 * and rejects ambiguous expansions before applying the redirect.
 */
static int	process_target(t_redirect *r, t_var *env, int status)
{
	char	*exp;
	char	**m;

	if (r->type == HEREDOC)
		return (0);
	exp = ft_expand(r->target, env, status);
	if (!exp || (exp[0] == '\0' && !has_quotes(r->target)))
		return (ambiguous_redir_error(r->target, exp, NULL));
	if (has_unquoted_star(r->target))
	{
		m = collect_matches(exp);
		if (m && m[0] && m[1])
			return (ambiguous_redir_error(r->target, exp, m));
		if (m && m[0])
		{
			free(exp);
			exp = ft_strdup(m[0]);
		}
		if (m)
			ft_free_tab(m);
	}
	free(r->target);
	r->target = exp;
	return (0);
}

/*
 * Applies every redirection attached to a command by expanding targets,
 * opening the required file descriptors, and redirecting stdin or stdout.
 */
int	apply_redirections(t_node *cmd_node, t_var *env, int status)
{
	t_redirect	*r;
	int			fd;

	if (!cmd_node)
		return (0);
	r = cmd_node->redirect;
	while (r)
	{
		if (process_target(r, env, status) != 0)
			return (1);
		fd = open_redir(r);
		if (fd < 0)
			return (perror_redir(r->target));
		if (r->type == REDIR_IN || r->type == HEREDOC)
			dup2(fd, STDIN_FILENO);
		else
			dup2(fd, STDOUT_FILENO);
		close(fd);
		if (r->type == HEREDOC)
			r->heredoc_fd = -1;
		r = r->next;
	}
	return (0);
}
