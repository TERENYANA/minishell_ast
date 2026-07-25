#include "../minishell.h"

static int	count_vars(t_var *env)
{
	int	n;

	n = 0;
	while (env)
	{
		n++;
		env = env->next;
	}
	return (n);
}

static void	sort_ptrs(t_var **arr, int n)
{
	int		i;
	int		j;
	t_var	*tmp;

	i = 0;
	while (i < n - 1)
	{
		j = 0;
		while (j < n - 1 - i)
		{
			if (ft_strcmp(arr[j]->name, arr[j + 1]->name) > 0)
			{
				tmp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = tmp;
			}
			j++;
		}
		i++;
	}
}

static int	print_export(t_var *env)
{
	t_var	**arr;
	int		n;
	int		i;

	n = count_vars(env);
	if (n == 0)
		return (0);
	arr = malloc(sizeof(t_var *) * n);
	if (!arr)
		return (1);
	i = 0;
	while (env)
	{
		arr[i++] = env;
		env = env->next;
	}
	sort_ptrs(arr, n);
	i = 0;
	while (i < n)
	{
		if (arr[i]->value)
		{
			ft_putstr_fd("declare -x ", STDOUT_FILENO);
			ft_putstr_fd(arr[i]->name, STDOUT_FILENO);
			ft_putstr_fd("=\"", STDOUT_FILENO);
			ft_putstr_fd(arr[i]->value, STDOUT_FILENO);
			ft_putendl_fd("\"", STDOUT_FILENO);
		}
		else
		{
			ft_putstr_fd("declare -x ", STDOUT_FILENO);
			ft_putendl_fd(arr[i]->name, STDOUT_FILENO);
		}
		i++;
	}
	free(arr);
	return (0);
}

static int	handle_export_arg(char *arg, t_var **env_list)
{
	char	*name;
	char	*value;

	if (parse_export_arg(arg, &name, &value))
		return (1);
	if (valid_name(name))
	{
		err_export(arg);
		free(name);
		free(value);
		return (1);
	}
	env_set(env_list, name, value);
	free(name);
	return (0);
}

int	ft_export(char **args, t_var **env_list)
{
	int i;
	int error;
	int end_opt;
	int ret;

	if (!args[1])
		return (print_export(*env_list));
	i = 1;
	error = 0;
	end_opt = 0;
	while (args[i])
	{
		ret = check_option(args[i], &end_opt);
		if (ret == 2)
			return (2);
		if (ret == 1)
		{
			i++;
			continue ;
		}
		if (handle_export_arg(args[i], env_list))
			error = 1;
		i++;
	}
	return (error);
}