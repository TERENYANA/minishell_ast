/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/25 20:43:00 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/25 20:43:00 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

char	*append_char(char *res, char c)
{
	int		len;
	char	*new;

	if (!res)
		return (NULL);
	len = ft_strlen(res);
	new = malloc(len + 2);
	if (!new)
		return (free(res), NULL);
	ft_memcpy(new, res, len);
	new[len] = c;
	new[len + 1] = '\0';
	free(res);
	return (new);
}
