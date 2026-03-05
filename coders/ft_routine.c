/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_routine.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:25:40 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/04 16:02:13 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	single_coder_routine(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	usleep(sim->time_to_burnout * 1000);
	if (!sim_is_over(sim))
	{
		log_action(sim, coder->id, "burned out");
		set_sim_over(sim);
		wake_all_dongles(sim);
	}
}

static void	stop_simulation(t_sim *sim)
{
	if (set_sim_over(sim))
	{
		usleep(5000);
		log_action(sim, 0, "all done - no burnout");
	}
	wake_all_dongles(sim);
	pthread_mutex_lock(&sim->monitor_mutex);
	pthread_cond_signal(&sim->monitor_cond);
	pthread_mutex_unlock(&sim->monitor_mutex);
}

static int	do_cycle(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	if (!do_compile(coder))
		return (0);
	if (sim_is_over(sim))
		return (0);
	if (all_done(sim))
	{
		stop_simulation(sim);
		return (0);
	}
	if (sim_is_over(sim))
		return (0);
	log_action(sim, coder->id, "is debugging");
	usleep(sim->time_to_debug * 1000);
	if (sim_is_over(sim))
		return (0);
	log_action(sim, coder->id, "is refactoring");
	usleep(sim->time_to_refactor * 1000);
	return (1);
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
		if (!do_cycle(coder))
			return (NULL);
	}
	return (NULL);
}
