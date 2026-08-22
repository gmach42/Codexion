/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heapq.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 00:00:00 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 20:04:00 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	heap_swap(t_hnode *a, t_hnode *b)
{
	t_hnode	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

/**
 * @brief Compare two heap nodes.
 *		Returns < 0 if `a` has strictly higher priority than `b`.
 *		Primary key  : priority (lower value = higher priority).
 *		Secondary key: tiebreak (arrival time, lower = earlier).
 *		Tertiary key : coder_id (lower = higher priority).
 */
static int	heap_cmp(const t_hnode *a, const t_hnode *b)
{
	if (a->priority != b->priority)
	{
		if (a->priority < b->priority)
			return (-1);
		return (1);
	}
	if (a->tiebreak != b->tiebreak)
	{
		if (a->tiebreak < b->tiebreak)
			return (-1);
		return (1);
	}
	if (a->coder_id < b->coder_id)
		return (-1);
	return (1);
}

void	heap_push(t_heap *heap, t_hnode node)
{
	int	i;
	int	parent;

	if (heap->size >= heap->capacity)
		return ;
	heap->nodes[heap->size] = node;
	i = heap->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (heap_cmp(&heap->nodes[i], &heap->nodes[parent]) >= 0)
			break ;
		heap_swap(&heap->nodes[i], &heap->nodes[parent]);
		i = parent;
	}
}

t_hnode	heap_pop(t_heap *heap)
{
	t_hnode	top;
	int		i;
	int		child;

	top = heap->nodes[0];
	heap->nodes[0] = heap->nodes[--heap->size];
	i = 0;
	while (1)
	{
		child = 2 * i + 1;
		if (child >= heap->size)
			break ;
		if (child + 1 < heap->size
			&& heap_cmp(&heap->nodes[child + 1], &heap->nodes[child]) < 0)
			child++;
		if (heap_cmp(&heap->nodes[i], &heap->nodes[child]) <= 0)
			break ;
		heap_swap(&heap->nodes[i], &heap->nodes[child]);
		i = child;
	}
	return (top);
}

static t_hnode	*heap_find(t_heap *heap, int coder_id)
{
	int	i;

	i = 0;
	while (i < heap->size)
	{
		if (heap->nodes[i].coder_id == coder_id)
			return (&heap->nodes[i]);
		i++;
	}
	return (NULL);
}

/**
 * @brief Insert `node` the first time this coder_id is seen on this
 *        heap, otherwise update its priority/tiebreak/pending in place.
 *        A dongle only ever has its 2 ring neighbors as coder_id, so
 *        each is created once and reused for the rest of the run.
 */
void	heap_upsert(t_heap *heap, t_hnode node)
{
	t_hnode	*existing;

	existing = heap_find(heap, node.coder_id);
	if (existing)
		*existing = node;
	else
		heap_push(heap, node);
}

void	heap_set_pending(t_heap *heap, int coder_id, bool pending)
{
	t_hnode	*node;

	node = heap_find(heap, coder_id);
	if (node)
		node->pending = pending;
}

/**
 * @brief True if `coder_id` has the best priority among the nodes
 *        currently marked pending (actively waiting). A node that is
 *        not pending (its owner is off compiling/debugging elsewhere)
 *        never counts, so it can't block a neighbor who really is
 *        waiting right now.
 */
bool	heap_pending_top_is(t_heap *heap, int coder_id)
{
	t_hnode	*best;
	int		i;

	best = NULL;
	i = 0;
	while (i < heap->size)
	{
		if (heap->nodes[i].pending
			&& (!best || heap_cmp(&heap->nodes[i], best) < 0))
			best = &heap->nodes[i];
		i++;
	}
	return (best != NULL && best->coder_id == coder_id);
}
