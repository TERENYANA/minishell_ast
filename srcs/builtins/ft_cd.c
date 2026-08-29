/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:14:48 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:14:48 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
 * Resolves the HOME directory for a cd command and reports when it is unset.
 */
static char	*cd_home(t_var *env, int *no_op)
{
	char	*path;

	*no_op = 0;
	path = get_env_value("HOME", env);
	if (!path)
	{
		ft_putstr_fd("minishell: cd: HOME not set\n", 2);
		return (NULL);
	}
	if (!*path)
	{
		*no_op = 1;
		return (NULL);
	}
	return (path);
}

/*
 * Resolves the previous directory for cd - and prints it when available.
 */
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

/*
 * Chooses the target directory for cd, including HOME and OLDPWD shortcuts.
 */
static char	*get_target_path(char **args, t_var *env, int *should_print,
		int *no_op)
{
	*should_print = 0;
	*no_op = 0;
	if (!args[1] || ft_strcmp(args[1], "~") == 0)
		return (cd_home(env, no_op));
	if (ft_strcmp(args[1], "-") == 0)
		return (cd_oldpwd(env, should_print));
	return (args[1]);
}

/*
 * Changes the working directory, updates PWD/OLDPWD, and optionally prints
 * the new path when cd - is used.
 */
static int	execute_cd(t_var **env_list, char *target_path, int should_print)
{
	char	*old_pwd;
	char	*current_pwd;

	old_pwd = getcwd(NULL, 0);
	if (chdir(target_path) != 0)
	{
		perror("minishell: cd");
		free(old_pwd);
		return (1);
	}
	if (should_print)
		ft_putendl_fd(target_path, STDOUT_FILENO);
	current_pwd = getcwd(NULL, 0);
	if (!current_pwd)
	{
		free(old_pwd);
		return (0);
	}
	if (old_pwd)
		env_set(env_list, "OLDPWD", old_pwd);
	env_set(env_list, "PWD", current_pwd);
	return (0);
}

/*
 * Implements the shell builtin cd: validates arguments, resolves the target,
 * performs the directory change, and updates environment variables.
 */
int	ft_cd(char **args, t_var **env_list)
{
	char	*target_path;
	int		should_print;
	int		no_op;

	if (args[1] && args[2])
	{
		ft_putstr_fd("minishell: cd: too many arguments\n", 2);
		return (2);
	}
	if (args[1] && args[1][0] == '\0')
		return (0);
	target_path = get_target_path(args, *env_list, &should_print, &no_op);
	if (no_op)
		return (0);
	if (!target_path)
		return (1);
	return (execute_cd(env_list, target_path, should_print));
}
