#include "../minishell.h"

static int	print_export(t_var *env)
{
	while (env)
	{
		if (env->value)
			printf("declare -x %s=\"%s\"\n", env->name, env->value);
		else
			printf("declare -x %s\n", env->name);
		env = env->next;
	}
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
	int	i;
	int	error;
	int	end_opt;
	int	ret;

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
		if (ret == 0 && handle_export_arg(args[i], env_list))
			error = 1;
		i++;
	}
	return (error);
}
