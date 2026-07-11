/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 14:50:01 by masolet-          #+#    #+#             */
/*   Updated: 2025/11/24 16:59:13 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_printf_c(char c, int fd, int *len)
{
	if (ft_putchar_fd2(c, fd) != -1 && *len >= 0)
		(*len)++;
	else
		(*len) = -1;
}

char	*ft_printf_s(const char *src)
{
	char	*dest;

	if (src == NULL)
		dest = ft_strdup("(null)");
	else
		dest = ft_strdup(src);
	return (dest);
}

char	*ft_printf_p(long n)
{
	char	*dest;
	char	*last;

	if (n == 0)
		return (ft_strdup("(nil)"));
	last = ft_ltoa_hexa(n, 'x');
	if (!last)
		return (NULL);
	dest = ft_strjoin("0x", last);
	if (!dest)
		return (NULL);
	free(last);
	return (dest);
}

int	ft_putstr_fd2(char *s, int fd)
{
	if (!s)
		return (-1);
	return (write(fd, s, ft_strlen(s)));
}

int	ft_putchar_fd2(char c, int fd)
{
	return (write(fd, &c, 1));
}
