/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_init.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:02:50 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/04 14:52:15 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongles(t_sim *sim)
{
	int	i;

	sim->dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->dongles[i].id = i;
		sim->dongles[i].in_use = 0;
		sim->dongles[i].available_at = 0;
		if (pthread_mutex_init(&sim->dongles[i].mutex, NULL) != 0)
			return (0);
		if (pthread_cond_init(&sim->dongles[i].cond, NULL) != 0)
			return (0);
		if (!pq_init(&sim->dongles[i].queue, sim->number_of_coders))
			return (0);
		i++;
	}
	return (1);
}

static int	init_coders(t_sim *sim)
{
	int	i;

	sim->coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!sim->coders)
		return (0);
	i = 0;
	while (i < sim->number_of_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_start = 0;
		sim->coders[i].deadline = sim->start_time + sim->time_to_burnout;
		sim->coders[i].left = &sim->dongles[i];
		sim->coders[i].right = &sim->dongles[(i + 1) % sim->number_of_coders];
		sim->coders[i].sim = sim;
		i++;
	}
	return (1);
}

int	ft_init(t_sim *sim)
{
	sim->start_time = get_time_ms();
	sim->simulation_over = 0;
	sim->coders = NULL;
	sim->dongles = NULL;
	if (pthread_mutex_init(&sim->print_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->sim_mutex, NULL) != 0)
		return (0);
	if (pthread_mutex_init(&sim->monitor_mutex, NULL) != 0)
		return (0);
	if (pthread_cond_init(&sim->monitor_cond, NULL) != 0)
		return (0);
	if (!init_dongles(sim))
		return (0);
	if (!init_coders(sim))
		return (0);
	return (1);
}
