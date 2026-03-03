/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_coder.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:02:42 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/03 15:30:27 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	sim_is_over(t_sim *sim)
{
	int	over;

	pthread_mutex_lock(&sim->sim_mutex);
	over = sim->simulation_over;
	pthread_mutex_unlock(&sim->sim_mutex);
	return (over);
}

int	set_sim_over(t_sim *sim)
{
	int	was_set;

	pthread_mutex_lock(&sim->sim_mutex);
	was_set = sim->simulation_over;
	sim->simulation_over = 1;
	pthread_mutex_unlock(&sim->sim_mutex);
	return (was_set == 0);
}

void	coder_burnout(t_coder *coder)
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

int	all_done(t_sim *sim)
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
