NAME       := minishell

CC         := cc
CFLAGS     := -Wall -Wextra -Werror
READLINE_PREFIX := $(shell brew --prefix readline)

INCLUDES := -I. -Ilibft -I$(READLINE_PREFIX)/include
LDLIBS   := -L$(READLINE_PREFIX)/lib -lreadline -Llibft -lft

SRCS       := \
	srcs/main.c \
	srcs/signals.c \
	srcs/env_init.c \
	srcs/free.c \
	srcs/debug.c \
	srcs/env_get.c \
	srcs/env_set.c \
	srcs/error.c \
	srcs/lexer/lexer.c \
	srcs/lexer/token.c \
	srcs/parser/syntax.c

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