/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_action.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:51:38 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/03 15:51:56 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	enqueue_both(t_coder *coder, t_dongle *first, t_dongle *second,
				long long priority)
{
	pthread_mutex_lock(&first->mutex);
	pq_push(&first->queue, coder->id, priority);
	pthread_mutex_unlock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	pq_push(&second->queue, coder->id, priority);
	pthread_mutex_unlock(&second->mutex);
}

static long long	get_priority(t_coder *coder)
{
	t_sim		*sim;
	long long	priority;

	sim = coder->sim;
	if (sim->scheduler == FIFO)
		priority = get_time_ms();
	else
		priority = coder->deadline;
	return (priority);
}

static int	take_second_or_abort(t_coder *coder, t_dongle *first,
				t_dongle *second, long long deadline)
{
	t_sim	*sim;

	sim = coder->sim;
	if (sim_is_over(sim) || get_time_ms() >= deadline)
	{
		dongle_dequeue(second, coder);
		release_dongle(first, sim);
		if (get_time_ms() >= deadline)
			return (-1);
		return (0);
	}
	if (!take_dongle_queued(second, coder, deadline))
	{
		release_dongle(first, sim);
		return (0);
	}
	return (1);
}

int	try_take_both(t_coder *coder, t_dongle *first, t_dongle *second)
{
	t_sim		*sim;
	long long	deadline;
	long long	priority;

	sim = coder->sim;
	pthread_mutex_lock(&sim->sim_mutex);
	deadline = coder->deadline;
	priority = get_priority(coder);
	pthread_mutex_unlock(&sim->sim_mutex);
	if (get_time_ms() >= deadline)
		return (-1);
	enqueue_both(coder, first, second, priority);
	if (!take_dongle_queued(first, coder, deadline))
	{
		dongle_dequeue(second, coder);
		return (0);
	}
	return (take_second_or_abort(coder, first, second, deadline));
}
