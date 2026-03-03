/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_compile.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:22:55 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/03 15:52:10 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	set_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	t_sim	*sim;

	sim = coder->sim;
	if (coder->id == sim->number_of_coders)
	{
		*first = coder->right;
		*second = coder->left;
	}
	else
	{
		*first = coder->left;
		*second = coder->right;
	}
}

static int	acquire_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	int	result;

	while (!sim_is_over(coder->sim))
	{
		result = try_take_both(coder, first, second);
		if (result == 1)
			return (1);
		if (result == -1)
			return (0);
		if (sim_is_over(coder->sim))
			return (0);
		usleep(1000);
	}
	return (0);
}

static void	update_deadline(t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->sim_mutex);
	coder->last_compile_start = get_time_ms();
	coder->deadline = coder->last_compile_start + sim->time_to_burnout;
	pthread_mutex_unlock(&sim->sim_mutex);
	pthread_mutex_lock(&sim->monitor_mutex);
	pthread_cond_signal(&sim->monitor_cond);
	pthread_mutex_unlock(&sim->monitor_mutex);
}

int	do_compile(t_coder *coder)
{
	t_sim		*sim;
	t_dongle	*first;
	t_dongle	*second;

	sim = coder->sim;
	set_dongles(coder, &first, &second);
	if (!acquire_dongles(coder, first, second))
		return (0);
	update_deadline(coder);
	log_action(sim, coder->id, "is compiling");
	usleep(sim->time_to_compile * 1000);
	release_dongle(first, sim);
	release_dongle(second, sim);
	pthread_mutex_lock(&sim->sim_mutex);
	coder->compile_count++;
	pthread_mutex_unlock(&sim->sim_mutex);
	return (1);
}
