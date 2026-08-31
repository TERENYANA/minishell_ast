/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals_heredoc.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 14:12:01 by masolet-          #+#    #+#             */
/*   Updated: 2026/08/28 14:12:01 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void    on_signal_heredoc(int signo)
{
	g_sig = signo;
	write(STDOUT_FILENO, "\n", 1);
	close(STDIN_FILENO);
}

void    setup_heredoc_signals(void)
{
    struct sigaction    sa;

	g_sig = 0;
	sa.sa_handler = on_signal_heredoc;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}
