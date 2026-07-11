/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/19 14:50:09 by masolet-          #+#    #+#             */
/*   Updated: 2025/11/24 17:08:32 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_putlen(const char *c, int i, int *len)
{
	if (write(1, &c[i], 1) == -1)
		(*len) = -1;
	else if (*len >= 0)
		(*len)++;
}

void	ft_put_and_free(char *argc, int *len)
{
	if (argc)
	{
		if (ft_putstr_fd2(argc, 1) != -1 && *len >= 0)
			*len += ft_strlen((const char *)argc);
		else
			*len = -1;
		free(argc);
	}
}

static int	ft_get_str(const char *format, int i, int *len, va_list *ap)
{
	char	c;
	char	*argc;

	argc = NULL;
	c = format[i + 1];
	if (!c)
		return (0);
	if (c == 'c')
		ft_printf_c(va_arg(*ap, int), 1, len);
	else if (c == '%')
		argc = ft_strdup("%");
	else if (c == 's')
		argc = ft_printf_s(va_arg(*ap, const char *));
	else if (c == 'x' || c == 'X')
		argc = ft_ltoa_hexa(va_arg(*ap, unsigned int), c);
	else if (c == 'p')
		argc = ft_printf_p(va_arg(*ap, unsigned long long));
	else if (c == 'd' || c == 'i')
		argc = ft_itoa(va_arg(*ap, int));
	else if (c == 'u')
		argc = ft_utoa(va_arg(*ap, unsigned int));
	else
		argc = ft_substr(format, i, 2);
	ft_put_and_free(argc, len);
	return (1);
}

int	ft_printf(const char *format, ...)
{
	int		i;
	int		len;
	va_list	ap;

	if (format == 0)
		return (-1);
	i = 0;
	len = 0;
	va_start(ap, format);
	while (format[i])
	{
		if (format[i] == '%')
		{
			if (!ft_get_str(format, i, &len, &ap))
				return (-1);
			i++;
		}
		else
			ft_putlen(format, i, &len);
		i++;
	}
	va_end(ap);
	return (len);
}
