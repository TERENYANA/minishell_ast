/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   child_exec_utils.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/29 13:35:00 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/29 13:35:00 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/* Returns 1 if path is a directory, 0 otherwise. */
static int	is_directory(const char *path)
{
	struct stat	st;

	if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		return (1);
	return (0);
}

/* Resolves the full path of a command or exits with 127/126.
** - No path found    → "command not found"  → exit 127
** - Path is a dir   → "is a directory"      → exit 126 */
static char	*cmd_path_or_exit(t_node *root, t_node *cur, t_var **env)
{
	char	*path;

	path = find_cmd_path(cur->cmd[0], *env);
	if (!path)
	{
		if (ft_strchr(cur->cmd[0], '/'))
			err_msg(cur->cmd[0], "No such file or directory", 0);
		else
			err_msg(cur->cmd[0], "command not found", 0);
		cleanup_and_exit(root, env, 127);
	}
	if (is_directory(path))
	{
		err_msg(cur->cmd[0], "is a directory", 0);
		free(path);
		cleanup_and_exit(root, env, 126);
	}
	return (path);
}

static char	**copy_envp(char **envp, char *entry, int len)
{
	char	**new_envp;
	int		i;

	new_envp = ft_calloc(len + 2, sizeof(char *));
	if (!new_envp)
		return (free(entry), envp);
	i = 0;
	while (i < len)
	{
		new_envp[i] = envp[i];
		i++;
	}
	new_envp[i] = entry;
	free(envp);
	return (new_envp);
}

/* Sets/replaces "_=<path>" in the envp array that is about to be
** handed to execve, WITHOUT touching the shell's own t_var list.
** This mirrors bash: '_' reflects the last executed command's path
** in the child's environment, but is never itself an exported
** shell variable (so it must never show up in export/env output). */
static char	**set_underscore(char **envp, const char *path)
{
	int		i;
	char	*entry;

	entry = ft_strjoin("_=", (char *)path);
	if (!entry)
		return (envp);
	i = 0;
	while (envp[i])
	{
		if (ft_strncmp(envp[i], "_=", 2) == 0)
		{
			free(envp[i]);
			envp[i] = entry;
			return (envp);
		}
		i++;
	}
	return (copy_envp(envp, entry, i));
}

/* Runs an external command via execve.
** convert_env_list builds the char** envp from our t_var list.
** set_underscore patches '_' only in this local envp copy, so the
** shell's own environment (and export/env output) stays untouched.
** execve never returns on success — code after it only runs on error. */
void	exec_external_cmd(t_node *root, t_node *cur, t_var **env)
{
	char	*path;
	char	**envp;
	int		exit_code;

	path = cmd_path_or_exit(root, cur, env);
	envp = convert_env_list(*env);
	if (!envp)
	{
		free(path);
		cleanup_and_exit(root, env, 1);
	}
	envp = set_underscore(envp, path);
	execve(path, cur->cmd, envp);
	perror(cur->cmd[0]);
	exit_code = 126;
	if (errno == ENOENT)
		exit_code = 127;
	free(path);
	ft_free_tab(envp);
	cleanup_and_exit(root, env, exit_code);
}
