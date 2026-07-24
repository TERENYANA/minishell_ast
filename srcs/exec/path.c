#include "../minishell.h"

/*
** FUNCTION: join_path
** -------------------
** Concatenates a directory path and a command name with a '/' in between.
**
** Param: dir - Directory path (e.g., "/usr/bin")
** Param: cmd - Command name (e.g., "ls")
**
** Return: Dynamically allocated string (e.g., "/usr/bin/ls"), or NULL on allocation failure.
**
** EXAMPLE TRACE:
** Input: dir = "/bin", cmd = "cat"
** 1. tmp = ft_strjoin("/bin", "/")  -> tmp becomes "/bin/"
** 2. res = ft_strjoin("/bin/", "cat") -> res becomes "/bin/cat"
** 3. free(tmp)                        -> Frees intermediate string "/bin/"
** 4. Returns "/bin/cat"
*/
static char	*join_path(const char *dir, const char *cmd)
{
	char	*tmp;
	char	*res;

	// Append trailing slash to directory name
	tmp = ft_strjoin((char *)dir, "/");
	if (!tmp)
		return (NULL);
		
	// Append command name to the directory path containing the slash
	res = ft_strjoin(tmp, (char *)cmd);
	
	// Free intermediate string to prevent memory leaks
	free(tmp);
	return (res);
}

/*
** FUNCTION: search_in_paths
** -------------------------
** Iterates through an array of directory paths to find where the executable command resides.
**
** Param: dirs - NULL-terminated array of directory path strings (e.g., ["/usr/bin", "/bin", NULL])
** Param: cmd  - Command name to locate (e.g., "grep")
**
** Return: Full path to executable (e.g., "/bin/grep") if found and executable, otherwise NULL.
**
** EXAMPLE TRACE:
** Input: dirs = ["/usr/bin", "/bin", NULL], cmd = "ls"
**
** Iteration 0:
**   - dirs[0] = "/usr/bin"
**   - full = join_path("/usr/bin", "ls") -> "/usr/bin/ls"
**   - access("/usr/bin/ls", X_OK) checks if file exists & is executable.
**   - Assuming true (0 returned): returns "/usr/bin/ls" immediately.
**
** If Iteration 0 failed (e.g., file not in /usr/bin):
**   - free(full) frees "/usr/bin/ls"
**   - Loop continues to Iteration 1 (dirs[1] = "/bin")
*/
static char	*search_in_paths(char **dirs, const char *cmd)
{
	char	*full;
	int		i;

	i = 0;
	while (dirs && dirs[i])
	{
		// Build path string: e.g., dirs[i] + "/" + cmd
		full = join_path(dirs[i], cmd);
		
		// Check if path exists AND process has executable rights (X_OK)
		if (full && access(full, X_OK) == 0)
			return (full);
			
		// If check failed, free allocated string before trying next directory
		free(full);
		i++;
	}
	return (NULL); // Command not found in any PATH directory
}

/*
** FUNCTION: find_cmd_path
** -----------------------
** Main entry point to resolve a command string into a valid executable path.
**
** Param: cmd - Input command string (e.g., "ls", "./script.sh", or "/bin/cat")
** Param: env - Pointer to environment variables list/structure
**
** Return: Dynamically allocated full path string, or NULL if command cannot be resolved.
**
** EXAMPLE TRACE 1 (Explicit path given):
**   cmd = "./a.out"
**   - ft_strchr("./a.out", '/') finds '/'
**   - Returns ft_strdup("./a.out") directly (skips PATH lookup).
**
** EXAMPLE TRACE 2 (Standard command lookup):
**   cmd = "ls", env contains PATH="/usr/bin:/bin"
**   1. Passes empty/null checks.
**   2. ft_strchr("ls", '/') returns NULL (no slash in command).
**   3. path = get_env_value("PATH", env) -> "usr/bin:/bin"
**   4. dirs = ft_split("usr/bin:/bin", ':') -> ["/usr/bin", "/bin", NULL]
**   5. search_in_paths(dirs, "ls") -> searches dirs and returns "/bin/ls"
**   6. ft_free_tab(dirs) frees the split array memory.
**   7. Returns "/bin/ls"
*/
char	*find_cmd_path(char *cmd, t_var *env)
{
	char	**dirs;
	char	*path;
	char	*found;

	// Guard clause: empty input check
	if (!cmd || !cmd[0])
		return (NULL);
		
	// If command contains a slash (e.g., "./a.out" or "/bin/ls"), return a duplicate
	if (ft_strchr(cmd, '/'))
		return (ft_strdup(cmd));
		
	// Extract PATH string from environment variables
	path = get_env_value("PATH", env);
	if (!path)
		return (NULL);
		
	// Split PATH string by ':' delimiter into array of paths
	dirs = ft_split(path, ':');
	if (!dirs)
		return (NULL);
		
	// Search array of directories for an executable file matching 'cmd'
	found = search_in_paths(dirs, cmd);
	
	// Clean up split array memory
	ft_free_tab(dirs);
	
	return (found);
}