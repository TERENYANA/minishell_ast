#include "../minishell.h"

int	ft_unset(char **args, t_var **env_list)
{
	int	i;

	if (!args[1])
		return (0);
	i = 1;
	while (args[i])
	{
		if (args[i][0] == '-' && args[i][1])
		{
			ft_putstr_fd("minishell: unset: ", 2);
			ft_putstr_fd(args[i], 2);
			ft_putstr_fd(": invalid option\n", 2);
			return (2);
		}
		env_unset(env_list, args[i]);
		i++;
	}
	return (0);
}
