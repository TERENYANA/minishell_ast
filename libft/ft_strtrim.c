/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/17 12:08:39 by masolet-          #+#    #+#             */
/*   Updated: 2025/11/17 12:09:12 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	is_inset(char c, const char *set)
{
	size_t	i;

	i = 0;
	while (set[i])
		if (set[i++] == c)
			return (1);
	return (0);
}

char	*ft_strtrim(char const *s1, char const *set)
{
	char	*trimmed;
	size_t	start;
	size_t	end;

	if (!s1 || !set)
		return (NULL);
	start = 0;
	while (is_inset(s1[start], set))
		start++;
	end = ft_strlen(s1) - 1;
	if (start == end + 1)
		return (ft_calloc(1, 1));
	while (is_inset(s1[end], set))
		end--;
	trimmed = ft_substr(s1, start, (end - start + 1));
	return (trimmed);
}
