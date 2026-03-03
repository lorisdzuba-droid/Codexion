#include "codexion.h"

static long long	find_earliest_deadline(t_sim *sim)
{
	long long	earliest;
	int			i;

	pthread_mutex_lock(&sim->sim_mutex);
	earliest = sim->coders[0].deadline;
	i = 1;
	while (i < sim->number_of_coders)
	{
		if (sim->coders[i].deadline < earliest)
			earliest = sim->coders[i].deadline;
		i++;
	}
	pthread_mutex_unlock(&sim->sim_mutex);
	return (earliest);
}

static int	find_burned_out(t_sim *sim)
{
	long long	now;
	int			i;

	now = get_time_ms();
	pthread_mutex_lock(&sim->sim_mutex);
	i = 0;
	while (i < sim->number_of_coders)
	{
		if (now >= sim->coders[i].deadline)
		{
			pthread_mutex_unlock(&sim->sim_mutex);
			return (sim->coders[i].id);
		}
		i++;
	}
	pthread_mutex_unlock(&sim->sim_mutex);
	return (-1);
}

void	wake_all_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

void	*monitor_routine(void *arg)
{
	t_sim			*sim;
	int				burned_id;
    long long		next_deadline;
	struct timespec	ts;

	sim = (t_sim *)arg;

	pthread_mutex_lock(&sim->monitor_mutex);
	while (1)
	{
		pthread_mutex_lock(&sim->sim_mutex);
		if (sim->simulation_over)
		{
			pthread_mutex_unlock(&sim->sim_mutex);
			break ;
		}
		pthread_mutex_unlock(&sim->sim_mutex);

		next_deadline = find_earliest_deadline(sim);
		ts = ms_to_timespec(next_deadline);
		pthread_cond_timedwait(&sim->monitor_cond, &sim->monitor_mutex, &ts);

		pthread_mutex_lock(&sim->sim_mutex);
		if (sim->simulation_over)
		{
			pthread_mutex_unlock(&sim->sim_mutex);
			break ;
		}
		pthread_mutex_unlock(&sim->sim_mutex);

		burned_id = find_burned_out(sim);
		if (burned_id != -1)
		{
			pthread_mutex_lock(&sim->sim_mutex);
			if (!sim->simulation_over)
			{
				sim->simulation_over = 1;
				pthread_mutex_unlock(&sim->sim_mutex);
				log_action(sim, burned_id, "burned out");
				wake_all_dongles(sim);
			}
			else
				pthread_mutex_unlock(&sim->sim_mutex);
			break ;
		}
	}
	pthread_mutex_unlock(&sim->monitor_mutex);
	return (NULL);
}