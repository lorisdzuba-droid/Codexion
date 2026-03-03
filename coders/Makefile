# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/03/03 15:03:16 by ldzuba            #+#    #+#              #
#    Updated: 2026/03/03 15:51:14 by ldzuba           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= codexion

CC		= gcc
CFLAGS	= -Wall -Wextra -Werror -pthread

SRCS	= codex.c \
		  ft_parsing.c \
		  ft_init.c \
		  ft_cleanup.c \
		  ft_utils.c \
		  ft_pqueue.c \
		  ft_pqueue_utils.c\
		  ft_dongle.c \
		  ft_dongle_next.c\
		  ft_coder.c \
		  ft_compile.c\
		  ft_monitor.c\
		  ft_action.c\
		  ft_routine.c

OBJS	= $(SRCS:.c=.o)

INCLUDES = -I coders/

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
