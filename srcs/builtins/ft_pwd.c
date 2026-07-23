#include "../minishell.h"

int	ft_pwd(char **args)
{
	char	*pwd;

	(void)args;
	pwd = getcwd(NULL, 0);
	if (!pwd)
	{
		perror("pwd");
		return (1);
	}
	printf("%s\n", pwd);
	free(pwd);
	return (0);
}
