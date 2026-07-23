#include "../minishell.h"

int	ft_strcmp(const char *a, const char *b)
{
	size_t	i;

	if (!a || !b)
	{
		if (a == b)
			return (0);
		if (a)
			return (1);
		return (-1);
	}
	i = 0;
	while (a[i] && a[i] == b[i])
		i++;
	return ((unsigned char)a[i] - (unsigned char)b[i]);
}

