#include "../minishell.h"

static char	*get_target_path(char **args, t_var *env)
{
	char	*path;

	if (args[1] && args[2])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", 2);
		return (NULL);
	}
	if (!args[1] || ft_strcmp(args[1], "~") == 0)
	{
		path = get_env_value("HOME", env);
		if (!path)
			ft_putstr_fd("minishell: cd: HOME not set\n", 2);
		return (path);
	}
	if (ft_strcmp(args[1], "-") == 0)
	{
		path = get_env_value("OLDPWD", env);
		if (!path)
			ft_putstr_fd("minishell: cd: OLDPWD not set\n", 2);
		else
			printf("%s\n", path);
		return (path);
	}
	return (args[1]);
}

int	ft_cd(char **args, t_var **env_list)
{
	char	*target_path;
	char	*old_pwd;
	char	*current_pwd;

	target_path = get_target_path(args, *env_list);
	if (!target_path)
		return (1);
	old_pwd = getcwd(NULL, 0);
	if (chdir(target_path) != 0)
	{
		perror("minishell: cd");
		free(old_pwd);
		return (1);
	}
	current_pwd = getcwd(NULL, 0);
	if (!current_pwd)
	{
		perror("minishell: cd: getcwd");
		free(old_pwd);
		return (1);
	}
	env_set(env_list, "OLDPWD", old_pwd);
	env_set(env_list, "PWD", current_pwd);
	return (0);
}
