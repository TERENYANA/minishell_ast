/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_conversions.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 14:50:12 by masolet-          #+#    #+#             */
/*   Updated: 2025/11/24 16:33:07 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_ltoa_hexa_size(unsigned long long nb)
{
	int	nlen;

	nlen = 0;
	while (nb > 0)
	{
		nb /= 16;
		nlen++;
	}
	return (nlen);
}

char	*ft_ltoa_hexa(unsigned long long n, char format)
{
	size_t	nlen;
	char	*res;

	if (n == 0)
		return (ft_strdup("0"));
	nlen = ft_ltoa_hexa_size(n);
	res = ft_calloc((nlen + 1) * sizeof(char), 1);
	if (!res)
		return (NULL);
	res[nlen] = '\0';
	while (n > 0)
	{
		nlen--;
		res[nlen] = "0123456789abcdef"[(n % 16)];
		if (format == 'X' && (res[nlen] >= 'a' && res[nlen] <= 'f'))
			res[nlen] -= 32;
		n /= 16;
	}
	return (res);
}

int	ft_utoa_size(long nb)
{
	int	nlen;

	nlen = 0;
	if (nb < 0)
	{
		nlen = 1;
		nb = -nb;
	}
	while (nb > 0)
	{
		nb /= 10;
		nlen++;
	}
	return (nlen);
}

char	*ft_utoa(unsigned int n)
{
	unsigned int	nlen;
	unsigned int	num;
	char			*res;

	if (n == 0)
		return (ft_strdup("0"));
	num = n;
	nlen = ft_utoa_size(num);
	res = malloc((nlen + 1) * sizeof(char));
	if (!res)
		return (NULL);
	res[nlen] = '\0';
	while (num > 0)
	{
		nlen--;
		res[nlen] = '0' + (num % 10);
		num /= 10;
	}
	return (res);
}
