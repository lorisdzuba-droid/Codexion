NAME	= codexion

CC		= gcc
CFLAGS	= -Wall -Wextra -Werror -pthread

SRCS	= codex.c \
		  ft_parsing.c \
		  ft_init.c \
		  ft_cleanup.c \
		  ft_utils.c \
		  ft_pqueue.c \
		  ft_dongle.c \
		  ft_coder.c \
		  ft_monitor.c

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
# ```

# ---

# Here's the full picture of every file and what it does:
# ```
# coders/
# ├── codexion.h       — all structs, enums, prototypes
# ├── codex.c          — main, launch_threads, join_threads
# ├── ft_parsing.c     — validate and fill t_sim from argv
# ├── ft_init.c        — alloc coders/dongles, init all mutexes/conds
# ├── ft_cleanup.c     — destroy mutexes/conds, free memory
# ├── ft_utils.c       — get_time_ms, log_action
# ├── ft_pqueue.c      — min-heap: push, pop, peek, remove, reheapify
# ├── ft_dongle.c      — take_dongle, release_dongle
# ├── ft_coder.c       — coder_routine, do_compile, burnout handling
# └── ft_monitor.c     — monitor_routine, wake_all_dongles
# ```

# ---

# The full flow end to end:
# ```
# main
#  ├── ft_parsing()        validates args → fills t_sim
#  ├── ft_init()           allocs everything, inits mutexes/conds
#  ├── launch_threads()
#  │    ├── monitor thread  sleeps until next deadline, logs burnout
#  │    └── N coder threads each loop: compile → debug → refactor
#  │         └── do_compile()
#  │              ├── take_dongle(left)   blocks in heap queue until turn
#  │              ├── take_dongle(right)  same
#  │              ├── usleep(compile)
#  │              ├── release both dongles (cooldown starts)
#  │              └── signals monitor with new deadline
#  ├── join_threads()      waits for all coders, then wakes+joins monitor
#  └── ft_cleanup()        frees everything