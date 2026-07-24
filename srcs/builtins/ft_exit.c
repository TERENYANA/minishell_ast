#include "../minishell.h"

static int	is_numeric_str(const char *s)
{
	int	i;

	if (!s || !*s)
		return (0);
	i = 0;
	if (s[i] == '+' || s[i] == '-')
		i++;
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (!ft_isdigit(s[i]))
			return (0);
		i++;
	}
	return (1);
}

static void	exit_numeric_err(const char *arg)
{
	ft_putstr_fd("minishell: exit: ", 2);
	ft_putstr_fd((char *)arg, 2);
	ft_putendl_fd(": numeric argument required", 2);
}

int	ft_exit(t_node *root, t_node *cur, t_var **env, int last_status)
{
	long	code;

	ft_putendl_fd("exit", STDERR_FILENO);
	if (!cur->cmd[1])
		cleanup_and_exit(root, env, last_status);
	if (!is_numeric_str(cur->cmd[1]))
	{
		exit_numeric_err(cur->cmd[1]);
		cleanup_and_exit(root, env, 2);
	}
	if (cur->cmd[2])
		return (err_msg("exit", "too many arguments", 1));
	code = ft_atoi(cur->cmd[1]);
	cleanup_and_exit(root, env, (unsigned char)code);
	return (0);
}