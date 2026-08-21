/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: masolet- <masolet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 16:08:20 by yyuskiv           #+#    #+#             */
/*   Updated: 2026/08/20 16:53:26 by masolet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <fcntl.h>
# include <limits.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <signal.h>
# include <errno.h>
# include <stdbool.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "libft.h"

void			rl_replace_line(const char *text, int clear_undo);

extern volatile sig_atomic_t	g_sig;

typedef enum e_token_type
{
	WORD = 0,
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	HEREDOC,
	PIPE,
	AND_IF,
	OR_IF,
	LPAREN,
	RPAREN
}	t_token_type;

typedef enum e_qstate
{
	Q_NONE,
	Q_SINGLE,
	Q_DOUBLE
}	t_qstate;

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

typedef struct s_var
{
	char			*name;
	char			*value;
	struct s_var	*next;
	struct s_var	*prev;
}	t_var;

typedef enum e_node_type
{
	N_CMD,
	N_PIPE,
	N_AND,
	N_OR,
	N_SUB
}	t_node_type;

typedef struct s_redirect
{
	t_token_type		type;
	int					expand_heredoc;
	int					heredoc_fd;
	char				*target;
	struct s_redirect	*next;
}	t_redirect;

typedef struct s_node
{
	t_node_type		type;
	char			**cmd;
	t_redirect		*redirect;
	struct s_node	*left;
	struct s_node	*right;
}	t_node;

typedef struct s_parse_info
{
	t_var	*env;
	int		status;
	int		*error_code;
}	t_parse_info;

typedef struct s_hd
{
	t_redirect	*redir;
	t_var		*env;
	int			status;
}	t_hd;

void			setup_signal_handlers(void);
void			ignore_signals(void);
void			setup_heredoc_signals(void);
void			setup_child_signals(void);

t_var			*create_env(char **envp);
t_var			*new_var(char *envp_line);
t_var			*find_var(t_var *env, const char *name);
char			*get_env_value(char *name, t_var *env);
char			**convert_env_list(t_var *env);
int				env_set(t_var **env, const char *name, char *value);
void			env_unset(t_var **env, const char *name);

t_token			*tokenize_line(char *line, int *err);
t_token			*create_token(char *value, t_token_type type);
t_token_type	assign_type(const char *s);
bool			add_token(char *value, t_token_type type, t_tok_list *list);
char			*extract_quoted(char *s, int len);
int				get_token_len(char *s);
int				is_special(char c);
int				is_whitespace(char c);
int				has_quotes(const char *s);
void			free_token_list(t_token *head);

int				syntax_ok(t_token *t, int *error_code);
int				syntax_err(const char *tok, int *error_code);
int				is_redir_tok(t_token_type t);
int				is_cmd_end(t_token *t);

t_node			*parsing(t_token *head, t_var *env, int status, int *err);
t_node			*parse_pipeline(t_token **cur, t_parse_info *info);
t_node			*parse_command(t_token **cur, t_parse_info *info);
t_node			*new_cmd_node(void);
t_node			*syntax_err_node(t_node *node, t_parse_info *info);
t_redirect		*create_redirect(t_token *op, t_token *target,
					t_parse_info *info);
int				add_arg(t_node *node, char *value);
int				add_word(t_node *node, t_token *tok, t_parse_info *info);
int				process_redir(t_node *cmd_node, t_token **cur,
					t_parse_info *info);
void			append_redirect(t_node *cmd_node, t_redirect *r);

t_node			*parse_list(t_token **cur, t_parse_info *info);
t_node			*parse_primary(t_token **cur, t_parse_info *info);
t_node			*new_op_node(t_node_type type, t_node *left, t_node *right);

char			*ft_expand(char *s, t_var *env, int status);
char			*append_char(char *res, char c);
int				handle_escape(char **res, char *s, int *i, t_qstate q);
char			*expand_heredoc_line(char *s, t_var *env, int status);

char			**collect_matches(const char *pattern);
int				has_unquoted_star(const char *s);
int				wc_match(const char *pat, const char *s);
int				tab_push(char ***tab, const char *name);
int				add_wildcard_args(t_node *node, char *pattern);
void			sort_tab(char **tab);

char			*find_cmd_path(char *cmd, t_var *env);
int				run_tree(t_node *root, t_var **env, int last_status);
int				run_line(char *line, t_var **env, int status);
int				handle_child_status(int wstatus);
int				apply_redirections(t_node *node);
int				fork_and_run(t_node *root, t_var **env);
void			exec_node_in_child(t_node *root, t_node *cur, t_var **env);
void			exec_external_cmd(t_node *root, t_node *cur, t_var **env);
void			exec_pipe_in_child(t_node *root, t_node *cur, t_var **env);
void			exec_andor_in_child(t_node *root, t_node *cur, t_var **env);
int				process_all_heredocs(t_node *node, t_var *env, int status);
void			heredoc_child(int p[2], t_hd *hd);
void			close_heredoc_fds(t_node *node);

int				is_builtin(char *cmd_name);
int				dispatch_builtin(t_node *node, t_var **env_list,
					int last_status);
int				ft_echo(char **args);
int				ft_pwd(char **args);
int				ft_env(t_var *env);
int				ft_export(char **args, t_var **env_list);
int				ft_cd(char **args, t_var **env_list);
int				ft_unset(char **args, t_var **env_list);
int				ft_exit(t_node *root, t_node *cur, t_var **env,
					int last_status);
int				valid_name(char *str);
int				parse_export_arg(char *arg, char **name, char **value);
int				check_option(char *arg, int *end_opt);
void			err_export(char *name);
void			sort_ptrs(t_var **arr, int n);
int				print_export(t_var *env);

void			ft_free_env(t_var *env);
void			ft_free_tab(char **tab);
void			ft_free_node(t_node *node);
void			ft_free_redirs(t_redirect *r);
void			cleanup_and_exit(t_node *root, t_var **env, int code);

int				ft_strcmp(const char *a, const char *b);
int				err_msg(const char *prefix, const char *msg, int code);

#endif
