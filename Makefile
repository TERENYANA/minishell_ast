NAME       := minishell

CC         := cc
CFLAGS     := -Wall -Wextra -Werror

UNAME_S    := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	READLINE_PREFIX := $(shell brew --prefix readline)
	INCLUDES := -I. -Iincludes -Ilibft -I$(READLINE_PREFIX)/include
	LDLIBS   := -L$(READLINE_PREFIX)/lib -lreadline -Llibft -lft
else
	INCLUDES := -I. -Iincludes -Ilibft
	LDLIBS   := -lreadline -Llibft -lft
endif

SRCS       = $(shell find srcs -type f -name "*.c")

OBJS       := $(patsubst srcs/%.c,objs/%.o,$(SRCS))
DEPS       := $(OBJS:.o=.d)

LIBFT      := libft/libft.a

all: $(NAME)

$(NAME): $(LIBFT) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDLIBS) -o $(NAME)

objs/%.o: srcs/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(LIBFT):
	$(MAKE) -C libft

clean:
	rm -f $(OBJS) $(DEPS)
	rm -rf objs
	$(MAKE) -C libft clean

fclean: clean
	rm -f $(NAME)
	$(MAKE) -C libft fclean

re: fclean all

-include $(DEPS)

.PHONY: all clean fclean re 