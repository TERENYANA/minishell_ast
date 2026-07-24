#include "../minishell.h"

int	handle_child_status(int wstatus)
{
	if (WIFSIGNALED(wstatus))
	{
		if (WTERMSIG(wstatus) == SIGQUIT)
			ft_putendl_fd("Quit (core dumped)", 2);
		return (128 + WTERMSIG(wstatus));
	}
	if (WIFEXITED(wstatus))
		return (WEXITSTATUS(wstatus));
	return (0);
}