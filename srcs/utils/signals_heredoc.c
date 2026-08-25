/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals_heredoc.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yyuskiv <yyuskiv@learner.42.tech>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:16:31 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/18 16:16:33 by yyuskiv          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static void	on_signal_heredoc(int signo)
{
	(void)signo;
	write(STDOUT_FILENO, "\n", 1);
	exit(130);
}

void	setup_heredoc_signals(void)
{
	struct sigaction	sa;

	sa.sa_handler = on_signal_heredoc;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}
