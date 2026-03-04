/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_monitor.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:02:58 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/04 15:34:44 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

static int	handle_burnout(t_sim *sim, int burned_id)
{
	pthread_mutex_lock(&sim->sim_mutex);
	if (sim->simulation_over)
	{
		pthread_mutex_unlock(&sim->sim_mutex);
		return (1);
	}
	sim->simulation_over = 1;
	pthread_mutex_unlock(&sim->sim_mutex);
	usleep(1000);
	log_action(sim, burned_id, "burned out");
	wake_all_dongles(sim);
	return (1);
}

static int	monitor_cycle(t_sim *sim)
{
	long long		next_deadline;
	struct timespec	ts;
	int				burned_id;

	pthread_mutex_lock(&sim->sim_mutex);
	if (sim->simulation_over)
	{
		pthread_mutex_unlock(&sim->sim_mutex);
		return (0);
	}
	pthread_mutex_unlock(&sim->sim_mutex);
	next_deadline = find_earliest_deadline(sim);
	ts = ms_to_timespec(next_deadline);
	pthread_cond_timedwait(&sim->monitor_cond, &sim->monitor_mutex, &ts);
	pthread_mutex_lock(&sim->sim_mutex);
	if (sim->simulation_over)
	{
		pthread_mutex_unlock(&sim->sim_mutex);
		return (0);
	}
	pthread_mutex_unlock(&sim->sim_mutex);
	burned_id = find_burned_out(sim);
	if (burned_id != -1)
		return (handle_burnout(sim, burned_id));
	return (1);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	pthread_mutex_lock(&sim->monitor_mutex);
	while (monitor_cycle(sim))
		;
	pthread_mutex_unlock(&sim->monitor_mutex);
	return (NULL);
}
