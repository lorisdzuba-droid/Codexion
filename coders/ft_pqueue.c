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