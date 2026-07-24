#include "../minishell.h"

/*
** FUNCTION: handle_child_status
** -----------------------------
** Translates a raw process wait status (retrieved from wait() or waitpid())
** into a standard POSIX shell exit status (0-255).
**
** Param: wstatus - The integer status populated by wait() or waitpid().
**
** Return: The normalized exit code for the shell.
**         - If process exited normally: returns its exit code (0 to 255).
**         - If process was terminated by a signal: returns (128 + signal_number)
**           and prints any required error messages to standard error (fd 2).
**
** WHAT THE POSIX MACROS DO:
** - WIFSIGNALED(status): Returns true if child was terminated by an unhandled signal.
** - WTERMSIG(status):   Extracts the specific signal number that killed the child.
** - WEXITSTATUS(status): Extracts the normal exit code passed to exit() by the child.
**
** EXAMPLE TRACE 1 (Normal exit):
**   A child process calls `exit(42);`.
**   1. waitpid(&wstatus) sets wstatus.
**   2. WIFSIGNALED(wstatus) is FALSE.
**   3. WEXITSTATUS(wstatus) extracts 42.
**   4. Function returns 42.
**
** EXAMPLE TRACE 2 (Terminated by Ctrl+C / SIGINT):
**   User presses Ctrl+C while a program like `cat` is running.
**   1. Child receives SIGINT (signal #2) and dies.
**   2. WIFSIGNALED(wstatus) is TRUE.
**   3. WTERMSIG(wstatus) yields 2 (SIGINT).
**   4. Writes a newline "\n" to stderr.
**   5. Returns 128 + 2 = 130 (standard shell exit code for SIGINT).
**
** EXAMPLE TRACE 3 (Terminated by Ctrl+\ / SIGQUIT):
**   User presses Ctrl+\ while a program is running.
**   1. Child receives SIGQUIT (signal #3) and dies with core dump.
**   2. WIFSIGNALED(wstatus) is TRUE.
**   3. WTERMSIG(wstatus) yields 3 (SIGQUIT).
**   4. Writes "Quit (core dumped)\n" to stderr (fd 2).
**   5. Returns 128 + 3 = 131 (standard shell exit code for SIGQUIT).
*/
int	handle_child_status(int wstatus)
{
	// Check if the child process was terminated abnormally by a signal
	if (WIFSIGNALED(wstatus))
	{
		// If killed by SIGQUIT (Ctrl+\), print core dump message to stderr
		if (WTERMSIG(wstatus) == SIGQUIT)
			write(2, "Quit (core dumped)\n", 19);
		// If killed by SIGINT (Ctrl+C), output a newline to clean up output
		else if (WTERMSIG(wstatus) == SIGINT)
			write(2, "\n", 1);
		
		// POSIX standard: exit code for signal termination is (128 + signal_number)
		return (128 + WTERMSIG(wstatus));
	}
	// If process exited normally (via exit() or return from main()), extract exit status
	return (WEXITSTATUS(wstatus));
}