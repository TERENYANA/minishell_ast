/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:15:30 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/31 18:55:31 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	ft_free_tab(char **tab)
{
	int	i;

	if (!tab)
		return ;
	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
}

void	ft_free_env(t_var *env)
{
	t_var	*next;

	while (env)
	{
		next = env->next;
		free(env->name);
		free(env->value);
		free(env);
		env = next;
	}
}

void	ft_free_redirs(t_redirect *r)
{
	t_redirect	*next;

	while (r)
	{
		next = r->next;
		if (r->type == HEREDOC && r->heredoc_fd >= 0)
			close(r->heredoc_fd);
		free(r->target);
		free(r);
		r = next;
	}
}

void	ft_free_node(t_node *node)
{
	if (!node)
		return ;
	if (node->type == N_CMD)
		ft_free_tab(node->cmd);
	else
	{
		ft_free_node(node->left);
		ft_free_node(node->right);
	}
	ft_free_redirs(node->redirect);
	free(node);
}

void	cleanup_and_exit(t_node *root, t_var **env, int code)
{
	ft_free_node(root);
	if (env && *env)
		ft_free_env(*env);
	get_next_line(-1);
	clear_history();
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	exit(code);
}
