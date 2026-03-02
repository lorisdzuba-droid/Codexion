#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
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
	WAITING_DONGLE
}	t_state;

// ─── Priority queue (min-heap) ───────────────────────────────────────────────

typedef struct s_pq_node
{
	long long	priority;	// arrival time (FIFO) or deadline (EDF)
	int			coder_id;
}	t_pq_node;

typedef struct s_pqueue
{
	t_pq_node	*nodes;
	int			size;
	int			capacity;
}	t_pqueue;

// ─── Dongle ──────────────────────────────────────────────────────────────────

typedef struct s_dongle
{
	int				id;
	int				in_use;
	long long		available_at;	// cooldown expiry timestamp (ms)
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	t_pqueue		queue;			// waiting coders (FIFO or EDF ordered)
}	t_dongle;

// ─── Coder ───────────────────────────────────────────────────────────────────

typedef struct s_coder
{
	int				id;
	t_state			state;
	int				compile_count;
	long long		last_compile_start;	// ms timestamp
	long long		deadline;			// last_compile_start + time_to_burnout
	t_dongle		*left;
	t_dongle		*right;
	pthread_t		thread;
	struct s_sim	*sim;
}	t_coder;

// ─── Simulation ──────────────────────────────────────────────────────────────

typedef struct s_sim
{
	// args
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
	int				simulation_over;
	pthread_mutex_t	print_mutex;
	pthread_mutex_t	sim_mutex;
	pthread_t		monitor;
    pthread_mutex_t	monitor_mutex;
    pthread_cond_t	monitor_cond;

}	t_sim;

// ─── Prototypes ──────────────────────────────────────────────────────────────

// parsing
int			ft_parsing(int argc, char **argv, t_sim *sim);

// init & cleanup
int			ft_init(t_sim *sim);
void		ft_cleanup(t_sim *sim);

// utils
long long	get_time_ms(void);
void		log_action(t_sim *sim, int coder_id, char *action);
struct timespec	ms_to_timespec(long long ms);

// priority queue
int			pq_init(t_pqueue *pq, int capacity);
void		pq_push(t_pqueue *pq, int coder_id, long long priority);
t_pq_node	pq_pop(t_pqueue *pq);
int			pq_peek_id(t_pqueue *pq);
void		pq_free(t_pqueue *pq);
void	    bubble_up(t_pqueue *pq, int i);
void	    bubble_down(t_pqueue *pq, int i);
void	    pq_reheapify(t_pqueue *pq, int i);

// dongle
void		release_dongle(t_dongle *dongle, t_sim *sim);
void	    dongle_enqueue(t_dongle *dongle, t_coder *coder, long long priority);
void	    dongle_dequeue(t_dongle *dongle, t_coder *coder);
int		    take_dongle_queued(t_dongle *dongle, t_coder *coder, long long deadline_ms);

// threads
void		*coder_routine(void *arg);
void		*monitor_routine(void *arg);

// monitoring
void	    *monitor_routine(void *arg);
void	    wake_all_dongles(t_sim *sim);


#endif