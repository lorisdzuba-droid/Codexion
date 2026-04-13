# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ldzuba <ldzuba@student.s19.be>             +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/27 12:15:36 by ldzuba            #+#    #+#              #
#    Updated: 2026/02/27 12:15:39 by ldzuba           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME	= codexion

CC		= gcc
CFLAGS	= -Wall -Wextra -Werror

SRCS	= codex.c ft_parsing.c

OBJS	= $(SRCS:.c=.o)

INCLUDES = -I codexion

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re