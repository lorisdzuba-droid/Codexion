/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pqueue.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:03:08 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/03 15:49:35 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	swap(t_pq_node *a, t_pq_node *b)
{
	t_pq_node	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	bubble_up(t_pqueue *pq, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (pq->nodes[parent].priority <= pq->nodes[i].priority)
			break ;
		swap(&pq->nodes[parent], &pq->nodes[i]);
		i = parent;
	}
}

void	bubble_down(t_pqueue *pq, int i)
{
	int	left;
	int	right;
	int	smallest;

	while (1)
	{
		left = 2 * i + 1;
		right = 2 * i + 2;
		smallest = i;
		if (left < pq->size
			&& pq->nodes[left].priority < pq->nodes[smallest].priority)
			smallest = left;
		if (right < pq->size
			&& pq->nodes[right].priority < pq->nodes[smallest].priority)
			smallest = right;
		if (smallest == i)
			break ;
		swap(&pq->nodes[i], &pq->nodes[smallest]);
		i = smallest;
	}
}

void	pq_push(t_pqueue *pq, int coder_id, long long priority)
{
	if (pq->size >= pq->capacity)
		return ;
	pq->nodes[pq->size].coder_id = coder_id;
	pq->nodes[pq->size].priority = priority;
	bubble_up(pq, pq->size);
	pq->size++;
}

t_pq_node	pq_pop(t_pqueue *pq)
{
	t_pq_node	top;

	top = pq->nodes[0];
	pq->nodes[0] = pq->nodes[pq->size - 1];
	pq->size--;
	bubble_down(pq, 0);
	return (top);
}
