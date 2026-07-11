#include "minishell.h"

volatile sig_atomic_t g_sig = 0;

static void on_signal_prompt(int signo)
{
    g_sig = signo;
    if (signo == SIGINT)
    {
        write(STDOUT_FILENO, "\n", 1);
        rl_on_new_line(); // Inform the readline library that we have moved to a new line. This is necessary to ensure that the prompt is displayed correctly after handling the signal.
        rl_replace_line("", 0); // Clear the current input line. The first argument is the new line (empty string in this case), and the second argument (0) indicates that we do not want to clear the undo history.
        rl_redisplay(); // Refresh the display to show the new prompt and the cleared input line. This will redraw the prompt and any input that was previously on the line, which is now cleared.
    }
}
void setup_signal_handlers(void)
{
    struct sigaction	sa; // Structure to hold the signal action

    sa.sa_handler = on_signal_prompt; // Set the signal handler function
    sigemptyset(&sa.sa_mask); // Initialize the signal mask to be empty (no signals are blocked during the execution of the handler)
    sa.sa_flags = 0; /* sans SA_RESTART : pour que readline soit interrompue */
    sigaction(SIGINT, &sa, NULL);
    signal(SIGQUIT, SIG_IGN);
}

void	ignore_signals(void)
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}
// Cette fonction désactive temporairement la réaction à Ctrl+C et Ctrl+\ pour le processus principal du shell.

/* Gestionnaire de heredoc : fermer stdin pour que readline dans la boucle heredoc
 * reçoive NULL. Sera nécessaire à la Phase 12. */
static void	on_signal_heredoc(int signo)
{
	g_sig = signo;
	close(STDIN_FILENO);
}// C'est le gestionnaire de Ctrl+C spécialement pour le mode Here-doc.

void	setup_heredoc_signals(void)
{
	struct sigaction	sa;

	sa.sa_handler = on_signal_heredoc;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	signal(SIGQUIT, SIG_IGN);
}