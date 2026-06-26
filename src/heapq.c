/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heapq.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/26 00:00:00 by gmach             #+#    #+#             */
/*   Updated: 2026/06/26 14:16:41 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void heap_swap(t_heap_node *a, t_heap_node *b)
{
	t_heap_node tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

/**
 * @brief Compare two heap nodes.
 *        Returns < 0 if `a` has strictly higher priority than `b`.
 *        Primary key  : priority (lower value = higher priority).
 *        Secondary key: tiebreak (arrival time, lower = earlier).
 *        Tertiary key : coder_id (lower = higher priority).
 */
static int heap_cmp(const t_heap_node *a, const t_heap_node *b)
{
	if (a->priority != b->priority)
		return ((a->priority < b->priority) ? -1 : 1);
	if (a->tiebreak != b->tiebreak)
		return ((a->tiebreak < b->tiebreak) ? -1 : 1);
	return ((a->coder_id < b->coder_id) ? -1 : 1);
}

/**
 * @brief Insert `node` into the min-heap and sift it up.
 *        No-op if the heap is already at capacity.
 */
void heap_push(t_heap *heap, t_heap_node node)
{
	int i;
	int parent;

	if (heap->size >= heap->capacity)
		return;
	heap->nodes[heap->size] = node;
	i = heap->size++;
	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (heap_cmp(&heap->nodes[i], &heap->nodes[parent]) >= 0)
			break;
		heap_swap(&heap->nodes[i], &heap->nodes[parent]);
		i = parent;
	}
}

/**
 * @brief Remove and return the minimum node, then sift the replacement down.
 *        Caller must ensure heap->size > 0 (use heap_top_is() first).
 */
t_heap_node heap_pop(t_heap *heap)
{
	t_heap_node top;
	int i;
	int child;

	top = heap->nodes[0];
	heap->nodes[0] = heap->nodes[--heap->size];
	i = 0;
	while (1)
	{
		child = 2 * i + 1;
		if (child >= heap->size)
			break;
		if (child + 1 < heap->size && heap_cmp(&heap->nodes[child + 1], &heap->nodes[child]) < 0)
			child++;
		if (heap_cmp(&heap->nodes[i], &heap->nodes[child]) <= 0)
			break;
		heap_swap(&heap->nodes[i], &heap->nodes[child]);
		i = child;
	}
	return (top);
}

/**
 * @brief Return true iff the heap is non-empty and its minimum node
 *        belongs to `coder_id`.
 */
bool heap_top_is(t_heap *heap, int coder_id)
{
	return (heap->size > 0 && heap->nodes[0].coder_id == coder_id);
}
