/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.s19.be>             +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/27 12:11:59 by ldzuba            #+#    #+#             */
/*   Updated: 2026/02/27 12:12:01 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>
# include <string.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef enum e_state
{
	COMPILING,
	DEBUGGING,
	REFACTORING,
	BURNED_OUT
}	t_state;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	int				available;
	long long		available_at;	// timestamp when cooldown expires
}	t_dongle;

typedef struct s_coder
{
	int				id;
	t_state			state;
	int				compile_count;
	long long		last_compile_start;	// timestamp, for burnout check
	long long		deadline;			// last_compile_start + time_to_burnout (for EDF)
	t_dongle		*left_dongle;
	t_dongle		*right_dongle;
	pthread_t		thread;
	struct s_sim	*sim;				// back pointer to shared simulation
}	t_coder;

typedef struct s_sim
{
	// parsed args
	int				number_of_coders;
	long long		time_to_burnout;
	long long		time_to_compile;
	long long		time_to_debug;
	long long		time_to_refactor;
	int				number_of_compiles_required;
	long long		dongle_cooldown;
	t_scheduler		scheduler;

	// runtime
	t_coder			*coders;
	t_dongle		*dongles;
	long long		start_time;
	int				simulation_over;	// flag checked by all threads
	pthread_mutex_t	print_mutex;		// so logs don't interleave
	pthread_mutex_t	sim_mutex;			// to protect simulation_over flag
}	t_sim;


int ft_parsing(int argc, char **argv, t_sim *sim);

# endif