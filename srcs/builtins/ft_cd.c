#include "../minishell.h"

static char	*cd_home(t_var *env)
{
	char	*path;

	path = get_env_value("HOME", env);
	if (!path || !*path)
	{
		ft_putstr_fd("minishell: cd: HOME not set\n", 2);
		return (NULL);
	}
	return (path);
}

static char	*cd_oldpwd(t_var *env, int *should_print)
{
	char	*path;

	path = get_env_value("OLDPWD", env);
	if (!path || !*path)
	{
		ft_putstr_fd("minishell: cd: OLDPWD not set\n", 2);
		return (NULL);
	}
	*should_print = 1;
	return (path);
}

static char	*get_target_path(char **args, t_var *env, int *should_print)
{
	*should_print = 0;
	if (!args[1] || ft_strcmp(args[1], "~") == 0)
		return (cd_home(env));
	if (ft_strcmp(args[1], "-") == 0)
		return (cd_oldpwd(env, should_print));
	return (args[1]);
}

static int	update_pwd(t_var **env_list, char *old_pwd)
{
	char	*current_pwd;

	current_pwd = getcwd(NULL, 0);
	if (!current_pwd)
	{
		perror("minishell: cd: getcwd");
		free(old_pwd);
		return (1);
	}
	if (old_pwd)
		env_set(env_list, "OLDPWD", old_pwd);
	env_set(env_list, "PWD", current_pwd);
	return (0);
}

int	ft_cd(char **args, t_var **env_list)
{
	char	*target_path;
	char	*old_pwd;
	int		should_print;

	target_path = get_target_path(args, *env_list, &should_print);
	if (!target_path)
		return (1);
	old_pwd = getcwd(NULL, 0);
	if (chdir(target_path) != 0)
	{
		perror("minishell: cd");
		free(old_pwd);
		return (1);
	}
	if (should_print)
		printf("%s\n", target_path);
	return (update_pwd(env_list, old_pwd));
}
