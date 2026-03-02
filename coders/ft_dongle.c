#include "codexion.h"

// Convert ms timestamp to struct timespec for pthread_cond_timedwait
struct timespec	ms_to_timespec(long long ms)
{
	struct timespec	ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	return (ts);
}

// Remove a coder from the middle of the heap (used on burnout bailout)
static void	pq_remove(t_pqueue *pq, int coder_id)
{
	int	i;

	i = 0;
	while (i < pq->size)
	{
		if (pq->nodes[i].coder_id == coder_id)
		{
			pq->nodes[i] = pq->nodes[pq->size - 1];
			pq->size--;
			// re-heapify: bubble up then down to cover both cases
			if (i > 0 && pq->nodes[i].priority
				< pq->nodes[(i - 1) / 2].priority)
			{
				// need bubble_up — but it's static in ft_pqueue.c
				// so we expose a pq_reheapify helper instead (see note below)
			}
			pq_reheapify(pq, i);
			return ;
		}
		i++;
	}
}

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

void	dongle_dequeue(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);
	pq_remove(&dongle->queue, coder->id);
	pthread_mutex_unlock(&dongle->mutex);
}

// Take dongle assuming already enqueued
int	take_dongle_queued(t_dongle *dongle, t_coder *coder, long long deadline_ms)
{
	long long		now;
	long long		wait_until;
	struct timespec	ts;
	t_sim			*sim;

	sim = coder->sim;
	pthread_mutex_lock(&dongle->mutex);
	while (1)
	{
		pthread_mutex_lock(&sim->sim_mutex);
		if (sim->simulation_over)
		{
			pthread_mutex_unlock(&sim->sim_mutex);
			pq_remove(&dongle->queue, coder->id);
			pthread_mutex_unlock(&dongle->mutex);
			return (0);
		}
		pthread_mutex_unlock(&sim->sim_mutex);

		now = get_time_ms();
		if (is_my_turn(dongle, coder->id, now))
		{
			dongle->in_use = 1;
			pq_pop(&dongle->queue);
			log_action(sim, coder->id, "has taken a dongle");
			pthread_mutex_unlock(&dongle->mutex);
			return (1);
		}
		if (now >= deadline_ms)
		{
			pq_remove(&dongle->queue, coder->id);
			pthread_mutex_unlock(&dongle->mutex);
			return (0);
		}
		wait_until = deadline_ms;
		if (!dongle->in_use && dongle->available_at > now
			&& dongle->available_at < wait_until)
			wait_until = dongle->available_at;
		ts = ms_to_timespec(wait_until);
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
	}
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