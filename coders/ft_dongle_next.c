/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_dongle_next.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:33:36 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/04 15:53:51 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	is_my_turn(t_dongle *dongle, int coder_id, long long now)
{
	if (dongle->in_use)
		return (0);
	if (now < dongle->available_at)
		return (0);
	if (pq_peek_id(&dongle->queue) != coder_id)
		return (0);
	return (1);
}

int	check_sim_over(t_dongle *dongle, t_coder *coder)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->sim_mutex);
	if (sim->simulation_over)
	{
		pthread_mutex_unlock(&sim->sim_mutex);
		pq_remove(&dongle->queue, coder->id);
		pthread_mutex_unlock(&dongle->mutex);
		return (1);
	}
	pthread_mutex_unlock(&sim->sim_mutex);
	return (0);
}

int	try_acquire(t_dongle *dongle, t_coder *coder, long long deadline_ms)
{
	long long	now;

	now = get_time_ms();
	if (is_my_turn(dongle, coder->id, now))
	{
		dongle->in_use = 1;
		pq_pop(&dongle->queue);
		log_action(coder->sim, coder->id, "has taken a dongle");
		pthread_mutex_unlock(&dongle->mutex);
		return (1);
	}
	if (now >= deadline_ms)
	{
		pq_remove(&dongle->queue, coder->id);
		pthread_mutex_unlock(&dongle->mutex);
		return (-1);
	}
	return (0);
}

void	wait_for_dongle(t_dongle *dongle, long long deadline_ms, int *wakeups)
{
	long long		now;
	long long		wait_until;
	struct timespec	ts;

	(*wakeups)++;
	now = get_time_ms();
	wait_until = deadline_ms;
	if (!dongle->in_use && dongle->available_at > now
		&& dongle->available_at < wait_until)
		wait_until = dongle->available_at;
	ts = ms_to_timespec(wait_until);
	pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
}

int	take_dongle_queued(t_dongle *dongle, t_coder *coder, long long deadline_ms)
{
	int	result;
	int	wakeups;

	wakeups = 0;
	pthread_mutex_lock(&dongle->mutex);
	while (1)
	{
		if (check_sim_over(dongle, coder))
			return (0);
		result = try_acquire(dongle, coder, deadline_ms);
		if (result == 1)
			return (1);
		if (result == -1)
		{
			return (0);
		}
		wait_for_dongle(dongle, deadline_ms, &wakeups);
	}
}
