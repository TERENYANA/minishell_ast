#include "../minishell.h"

int	is_redir_tok(t_token_type t)
{
	return (t == REDIR_IN || t == REDIR_OUT || t == REDIR_APPEND
		|| t == HEREDOC);
}
