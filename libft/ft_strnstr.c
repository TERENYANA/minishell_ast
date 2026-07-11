/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/17 11:42:22 by masolet-          #+#    #+#             */
/*   Updated: 2025/11/17 11:43:03 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *big, const char *little, size_t len)
{
	size_t	i;
	size_t	j;

	if (!*little)
		return ((char *)big);
	if (!len)
		return (NULL);
	i = ft_strlen(little);
	j = 0;
	while (ft_strncmp(&big[j], little, i))
	{
		if (!big[j] || j >= (len - i))
			return (NULL);
		j++;
	}
	if (i > len)
		return (NULL);
	return ((char *)&big[j]);
}
