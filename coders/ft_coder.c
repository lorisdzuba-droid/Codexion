#include "codexion.h"

static int	sim_is_over(t_sim *sim)
{
	int	over;

	pthread_mutex_lock(&sim->sim_mutex);
	over = sim->simulation_over;
	pthread_mutex_unlock(&sim->sim_mutex);
	return (over);
}

static int	set_sim_over(t_sim *sim)
{
	int	was_set;

	pthread_mutex_lock(&sim->sim_mutex);
	was_set = sim->simulation_over;
	sim->simulation_over = 1;
	pthread_mutex_unlock(&sim->sim_mutex);
	return (was_set == 0);
}


static void	coder_burnout(t_coder *coder)
{
	t_sim		*sim;
	long long	now;
	long long	remaining;

	sim = coder->sim;
	now = get_time_ms();
	remaining = coder->deadline - now;
	if (remaining > 0)
		usleep(remaining * 1000);
	if (set_sim_over(sim))
	{
		log_action(sim, coder->id, "burned out");
		wake_all_dongles(sim);
	}
}

static int	all_done(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->sim_mutex);
	i = 0;
	while (i < sim->number_of_coders)
	{
		if (sim->coders[i].compile_count < sim->number_of_compiles_required)
		{
			pthread_mutex_unlock(&sim->sim_mutex);
			return (0);
		}
		i++;
	}
	pthread_mutex_unlock(&sim->sim_mutex);
	return (1);
}

static int	try_take_both(t_coder *coder, t_dongle *first, t_dongle *second)
{
	t_sim		*sim;
	long long	deadline;
	long long	priority;

	sim = coder->sim;
	pthread_mutex_lock(&sim->sim_mutex);
	deadline = coder->deadline;
	if (sim->scheduler == FIFO)
		priority = get_time_ms();
	else
		priority = coder->deadline;
	pthread_mutex_unlock(&sim->sim_mutex);

	if (get_time_ms() >= deadline)
		return (-1);
	pthread_mutex_lock(&first->mutex);
	pq_push(&first->queue, coder->id, priority);
	pthread_mutex_unlock(&first->mutex);

	pthread_mutex_lock(&second->mutex);
	pq_push(&second->queue, coder->id, priority);
	pthread_mutex_unlock(&second->mutex);

	if (!take_dongle_queued(first, coder, deadline))
	{
		dongle_dequeue(second, coder);
		return (0);
	}
	if (sim_is_over(sim) || get_time_ms() >= deadline)
	{
		dongle_dequeue(second, coder);
		release_dongle(first, sim);
		return (get_time_ms() >= deadline ? -1 : 0);
	}

	if (!take_dongle_queued(second, coder, deadline))
	{
		release_dongle(first, sim);
		return (0);
	}
	return (1);
}


static int	do_compile(t_coder *coder)
{
	t_sim		*sim;
	t_dongle	*first;
	t_dongle	*second;
	int			result;

	sim = coder->sim;
	if (coder->id == sim->number_of_coders)
	{
		first = coder->right;
		second = coder->left;
	}
	else
	{
		first = coder->left;
		second = coder->right;
	}

	while (!sim_is_over(sim))
	{
		result = try_take_both(coder, first, second);
		if (result == 1)
			break ;
		if (result == -1)
			return (0);
		if (result == 0 && sim_is_over(sim))
			return (0);
		usleep(1000);
	}
	if (sim_is_over(sim))
		return (0);
	pthread_mutex_lock(&sim->sim_mutex);
	coder->last_compile_start = get_time_ms();
	coder->deadline = coder->last_compile_start + sim->time_to_burnout;
	pthread_mutex_unlock(&sim->sim_mutex);

	pthread_mutex_lock(&sim->monitor_mutex);
	pthread_cond_signal(&sim->monitor_cond);
	pthread_mutex_unlock(&sim->monitor_mutex);

	log_action(sim, coder->id, "is compiling");
	usleep(sim->time_to_compile * 1000);

	release_dongle(first, sim);
	release_dongle(second, sim);

	pthread_mutex_lock(&sim->sim_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&sim->sim_mutex);
	return (1);
}

static void	single_coder_routine(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	usleep(sim->time_to_burnout * 1000);
	if (!sim_is_over(sim))
		coder_burnout(coder);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;
	t_sim	*sim;

	coder = (t_coder *)arg;
	sim = coder->sim;

	if (sim->number_of_coders == 1)
	{
		single_coder_routine(coder);
		return (NULL);
	}
    if (coder->id % 2 == 0)
    usleep(sim->time_to_compile * 1000);

	while (!sim_is_over(sim))
    {
        if (!do_compile(coder))
        {
            return (NULL);
        }
        if (sim_is_over(sim))
            return (NULL);

		if (all_done(sim))
			{
				if (set_sim_over(sim))
					log_action(sim, 0, "all done - no burnout");
				wake_all_dongles(sim);
				pthread_mutex_lock(&sim->monitor_mutex);
				pthread_cond_signal(&sim->monitor_cond);
				pthread_mutex_unlock(&sim->monitor_mutex);
				return (NULL);
			}

        log_action(sim, coder->id, "is debugging");
        usleep(sim->time_to_debug * 1000);
        if (sim_is_over(sim))
            return (NULL);

        log_action(sim, coder->id, "is refactoring");
        usleep(sim->time_to_refactor * 1000);
    }
	return (NULL);
}
