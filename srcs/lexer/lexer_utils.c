#include "../minishell.h"

int	is_special(char c)
{
	return (c == '|' || c == '<' || c == '>'
		|| c == '&' || c == '(' || c == ')');
}

int	is_whitespace(char c)
{
	return (c == ' ' || c == '\t');
}
