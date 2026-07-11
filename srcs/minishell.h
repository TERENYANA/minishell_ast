#ifndef MINISHELL_H
# define MINISHELL_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <signal.h>
# include <errno.h>
# include <stdbool.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "libft.h"

/* Les structures et les prototypes sont ajoutés par phases. */
extern volatile sig_atomic_t g_sig;

void setup_signal_handlers(void);
void	ignore_signals(void);
void	setup_heredoc_signals(void);
void       rl_replace_line(const char *text, int clear_undo); //to delete, need only for OS

//LEXER
typedef enum e_token_type
{
	WORD = 0,
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	HEREDOC,
	PIPE
}	t_token_type;

typedef struct s_token
{
	char			*value;
	t_token_type	type;
	struct s_token	*next;
}	t_token;

typedef struct s_tok_list
{
	t_token	*head;
	t_token	*tail;
}	t_tok_list;

//LEXER

typedef struct s_var
{
    char *name;
    char *value;

    struct s_var *next;
    struct s_var *prev;
} t_var;


//envp

void	ft_free_env(t_var *env);

int		ft_strcmp(const char *a, const char *b);
t_var	*create_env(char **envp);
t_var	*new_var(char *envp_line);
char	*get_env_value(char *name, t_var *env);
char	**convert_env_list(t_var *env);
int		env_set(t_var **env, const char *name, char *value);
void	env_unset(t_var **env, const char *name);
t_var	*find_var(t_var *env, const char *name);
void	ft_free_env(t_var *env);
int		ft_strcmp(const char *a, const char *b);
void    ft_free_tab(char **tab);;

//envp
void print_env(t_var *e); //DELETE for debug
//LEXER
t_token	*tokenize_line(char *line, int *err);
int				get_token_len(char *s);
int				is_special(char c);
int				is_whitespace(char c);
t_token			*create_token(char *value, t_token_type type);
bool			add_token(char *value, t_token_type type, t_tok_list *list);
t_token_type	assign_type(const char *s);
char			*extract_quoted(char *s, int len);
int				has_quotes(const char *s);
void			free_token_list(t_token *head);
void	print_tokens(t_token *tokens);//DELETE TEST
//LEXER

//PARSER SYNTAX
int	syntax_ok(t_token *t, int *error_code);
int	syntax_err(const char *tok, int *error_code);

//PARSER

#endif



