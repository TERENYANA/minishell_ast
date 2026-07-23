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
	srcs/lexer/lexer_utils.c \
	srcs/lexer/token.c \
	srcs/lexer/token_utils.c \
	srcs/parser/syntax.c \
	srcs/parser/parser.c \
	srcs/parser/parser_redir.c \
	srcs/expand/expand.c \
	srcs/debug.c \
	srcs/expand/wildcard.c\
 	srcs/expand/wildcard_dir.c\
	srcs/expand/wildcard_apply.c

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