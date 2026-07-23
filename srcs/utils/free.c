#include "../minishell.h"

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
	{
		ft_free_tab(node->cmd);
		ft_free_redirs(node->redirect);
	}
	else
	{
		ft_free_node(node->left);
		ft_free_node(node->right);
	}
	free(node);
}

