/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_dongle.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:02:46 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/03 15:36:50 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

struct timespec	ms_to_timespec(long long ms)
{
	struct timespec	ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	return (ts);
}

void	pq_remove(t_pqueue *pq, int coder_id)
{
	int	i;

	i = 0;
	while (i < pq->size)
	{
		if (pq->nodes[i].coder_id == coder_id)
		{
			pq->nodes[i] = pq->nodes[pq->size - 1];
			pq->size--;
			pq_reheapify(pq, i);
			return ;
		}
		i++;
	}
}

void	dongle_dequeue(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);
	pq_remove(&dongle->queue, coder->id);
	pthread_mutex_unlock(&dongle->mutex);
}

void	release_dongle(t_dongle *dongle, t_sim *sim)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->in_use = 0;
	dongle->available_at = get_time_ms() + sim->dongle_cooldown;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	dongle_enqueue(t_dongle *dongle, t_coder *coder, long long priority)
{
	pthread_mutex_lock(&dongle->mutex);
	pq_push(&dongle->queue, coder->id, priority);
	pthread_mutex_unlock(&dongle->mutex);
}
