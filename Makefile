NAME       := minishell

CC         := cc
CFLAGS     := -Wall -Wextra -Werror
READLINE_PREFIX := $(shell brew --prefix readline)

INCLUDES := -I. -Ilibft -I$(READLINE_PREFIX)/include
LDLIBS   := -L$(READLINE_PREFIX)/lib -lreadline -Llibft -lft

SRCS       := \
	srcs/main.c \
	srcs/utils/signals.c \
	srcs/utils/free.c \
	srcs/utils/error.c \
	srcs/env/env_init.c \
	srcs/env/env_get.c \
	srcs/env/env_set.c \
	srcs/lexer/lexer.c \
	srcs/lexer/token.c \
	srcs/parser/syntax.c \
	srcs/parser/parser.c \
	srcs/parser/redir_parse.c \
	srcs/expand/expand.c \
	srcs/expand/expend_hd.c \
	srcs/builtins/builtins.c \
	srcs/builtins/ft_cd.c \
	srcs/builtins/ft_echo.c \
	srcs/builtins/ft_env.c \
	srcs/builtins/ft_exit.c \
	srcs/builtins/ft_export.c \
	srcs/builtins/ft_export_utils.c \
	srcs/builtins/ft_pwd.c \
	srcs/builtins/ft_unset.c \
	srcs/exec/heredoc.c \
	srcs/utils/debug.c

OBJS       := $(SRCS:.c=.o)
DEPS       := $(OBJS:.o=.d)

OBJS       := $(SRCS:.c=.o)
DEPS       := $(OBJS:.o=.d)

LIBFT      := libft/libft.a

all: $(NAME)

$(NAME): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDLIBS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(LIBFT):
	$(MAKE) -C libft

clean:
	rm -f $(OBJS) $(DEPS)
	$(MAKE) -C libft clean

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C libft fclean

re: fclean all

debug: CFLAGS += -g3 -fsanitize=address
debug: re

-include $(DEPS)

.PHONY: all clean fclean re debug