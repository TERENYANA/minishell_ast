#include "../minishell.h"

int	ft_unset(char **args, t_var **env_list)
{
	int	i;
	int	status;

	if (!args[1])
		return (0);
	i = 1;
	status = 0;
	while (args[i])
	{
		if (args[i][0] == '-' && args[i][1] != '\0')
		{
			ft_putstr_fd("minishell: unset: `", 2);
			ft_putstr_fd(args[i], 2);
			ft_putstr_fd("': invalid option\n", 2);
			return (2);
		}
		if (valid_name(args[i]) != 0)
		{
			// bash silently ignores invalid identifiers for unset
			// unless the -v option is explicitly provided.
		}
		else
			env_unset(env_list, args[i]);
		i++;
	}
	return (status);
}