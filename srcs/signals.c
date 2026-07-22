#include "minishell.h"

/* ** volatile: Prevents the compiler from caching the variable in CPU registers.
** sig_atomic_t: Guarantees that read/write operations on this variable are atomic
** (uninterruptible), even if a signal is caught mid-execution.
*/
volatile sig_atomic_t g_sig = 0;

/*
** Signal handler for interactive mode (when the shell prompt is active).
*/
static void on_signal_prompt(int signo)
{
    g_sig = signo;
    if (signo == SIGINT) // If Ctrl+C is pressed
    {
        write(STDOUT_FILENO, "\n", 1); // Move the cursor to a new line
        rl_on_new_line();  // Tell readline that the cursor has moved to a new line
        rl_replace_line("", 0); // Clear the current input buffer (discarding whatever the user typed)
        rl_redisplay(); // Force redisplay to show the empty prompt on the new line
    }
}

/*
** Sets up signal configurations for the interactive shell.
*/
void setup_signal_handlers(void)
{
    struct sigaction    sa; // Structure to configure signal behavior

    sa.sa_handler = on_signal_prompt; // Assign our handler function
    sigemptyset(&sa.sa_mask); // Clear the mask (no other signals are blocked during execution)
    
    /* ** Important: sa_flags = 0 (no SA_RESTART flag). This forces blocking system 
    ** calls (like read inside readline) to be interrupted and return EINTR, 
    ** allowing readline to break out of its input loop.
    */
    sa.sa_flags = 0; 
    sigaction(SIGINT, &sa, NULL); // Apply settings for Ctrl+C
    signal(SIGQUIT, SIG_IGN);     // Ignore Ctrl+\ (SIGQUIT) entirely at the prompt
}

/*
** Temporarily ignores signals in the parent process (the shell).
** Usually called right after fork() to ensure the shell doesn't respond to Ctrl+C/Ctrl+\ 
** while a child process (like cat or grep) is running and handling those signals itself.
*/
void    ignore_signals(void)
{
    signal(SIGINT, SIG_IGN);  // Ignore Ctrl+C , // Ignore Ctrl+\ after
    signal(SIGQUIT, SIG_IGN); 
}

/* 
** Signal handler for Ctrl+C specifically during Here-doc (<< EOF) processing.
*/
static void on_signal_heredoc(int signo)
{
    g_sig = signo;
    /*
    ** Trick: Close standard input (STDIN). 
    ** This causes readline()—which is currently waiting for input inside the heredoc—
    ** to immediately encounter an end-of-file (EOF) state and return NULL.
    ** This allows the heredoc loop to detect the break and terminate cleanly.
    */
    close(STDIN_FILENO);
}

/*
** Sets up signals specifically for the duration of the Here-doc collection phase.
*/
void    setup_heredoc_signals(void)
{
    struct sigaction    sa;

    sa.sa_handler = on_signal_heredoc; // Assign the heredoc-specific handler
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // No SA_RESTART, ensuring the read call is interrupted
    sigaction(SIGINT, &sa, NULL); // Apply for Ctrl+C
    signal(SIGQUIT, SIG_IGN);     // Ignore Ctrl+\ during heredoc input
}


/*If you use sa_flags = 0 (The correct way)
readline() is stuck waiting on read().

You press Ctrl+C.

The OS pauses read(), runs your signal handler, and executes your code.

Because sa_flags is 0, the OS says: "I will NOT restart the read() call."

Instead, the read() function instantly crashes with a special error: EINTR (Interrupted System Call).

readline() sees this error, stops trying to read, and returns control back to your main shell loop so you can print a brand-new, clean prompt.*/