/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_pqueue_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldzuba <ldzuba@student.42belgium.be>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/03 15:49:00 by ldzuba            #+#    #+#             */
/*   Updated: 2026/03/03 15:49:31 by ldzuba           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	pq_init(t_pqueue *pq, int capacity)
{
	pq->nodes = malloc(sizeof(t_pq_node) * capacity);
	if (!pq->nodes)
		return (0);
	pq->size = 0;
	pq->capacity = capacity;
	return (1);
}

void	pq_free(t_pqueue *pq)
{
	free(pq->nodes);
	pq->nodes = NULL;
	pq->size = 0;
}

int	pq_peek_id(t_pqueue *pq)
{
	if (pq->size == 0)
		return (-1);
	return (pq->nodes[0].coder_id);
}

void	pq_reheapify(t_pqueue *pq, int i)
{
	int	parent;

	parent = (i - 1) / 2;
	if (i > 0 && pq->nodes[i].priority < pq->nodes[parent].priority)
		bubble_up(pq, i);
	else
		bubble_down(pq, i);
}
