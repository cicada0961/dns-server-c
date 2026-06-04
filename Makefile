CC = gcc
CFLAGS = -Wall -Wextra

NAME = dns

SRCS = main.c src/socket.c src/table.c src/parser.c src/builder.c src/network.c src/utils.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.c dns.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
