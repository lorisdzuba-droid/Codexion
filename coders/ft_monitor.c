#include "codexion.h"

static t_coder	*find_earliest_deadline(t_sim *sim)
{
	t_coder	*earliest;
	int		i;

	earliest = &sim->coders[0];
	i = 1;
	while (i < sim->number_of_coders)
	{
		if (sim->coders[i].deadline < earliest->deadline)
			earliest = &sim->coders[i];
		i++;
	}
	return (earliest);
}

static t_coder	*find_burned_out(t_sim *sim)
{
	long long	now;
	int			i;

	now = get_time_ms();
	i = 0;
	while (i < sim->number_of_coders)
	{
		if (now >= sim->coders[i].deadline)
			return (&sim->coders[i]);
		i++;
	}
	return (NULL);
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
	t_coder			*target;
	t_coder			*burned;
	struct timespec	ts;

	sim = (t_sim *)arg;

	// Monitor has its own mutex+cond just for sleeping
	pthread_mutex_lock(&sim->monitor_mutex);
	while (1)
	{
		// Check if simulation already ended (all compiled)
		pthread_mutex_lock(&sim->sim_mutex);
		if (sim->simulation_over)
		{
			pthread_mutex_unlock(&sim->sim_mutex);
			break ;
		}
		pthread_mutex_unlock(&sim->sim_mutex);

		// Find who expires soonest and sleep until then
		target = find_earliest_deadline(sim);
		ts = ms_to_timespec(target->deadline);
		pthread_cond_timedwait(&sim->monitor_cond, &sim->monitor_mutex, &ts);

		// Woke up — check if sim ended while sleeping
		pthread_mutex_lock(&sim->sim_mutex);
		if (sim->simulation_over)
		{
			pthread_mutex_unlock(&sim->sim_mutex);
			break ;
		}
		pthread_mutex_unlock(&sim->sim_mutex);

		// Check for actual burnout
		burned = find_burned_out(sim);
        if (burned)
        {
            pthread_mutex_lock(&sim->sim_mutex);
            if (!sim->simulation_over)
            {
                sim->simulation_over = 1;
                pthread_mutex_unlock(&sim->sim_mutex);
                log_action(sim, burned->id, "burned out");
                wake_all_dongles(sim);
            }
            else
                pthread_mutex_unlock(&sim->sim_mutex);
            break ;
        }
		// No burnout yet (spurious wakeup or deadline updated) — loop again
	}
	pthread_mutex_unlock(&sim->monitor_mutex);
	return (NULL);
}