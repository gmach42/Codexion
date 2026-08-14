/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:04:03 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 09:00:42 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Build a heap node for coder requesting dongle.
 *        FIFO: priority = arrival time (ms).
 *        EDF : priority = last_compile_start + time_to_burnout (ms),
 *              tiebreak = arrival time.
 */
static t_heap_node build_node(t_sim *sim, t_coder *coder)
{
	t_heap_node node;

	node.coder_id = coder->id;
	node.tiebreak = get_current_time();
	if (sim->dongle_schedule == 0)
		node.priority = node.tiebreak;
	else
	{
		pthread_mutex_lock(&coder->time_mutex);
		node.priority = coder->last_compile_time + sim->time_to_burnout;
		pthread_mutex_unlock(&coder->time_mutex);
	}
	return (node);
}

/**
 * @brief Try to acquire dongle for a coder.
 *        Only takes the dongle when:
 *          - it is at the head of the priority queue, AND
 *          - the dongle is not in its cooldown period.
 *
 * @return true  on success (dongle taken).
 * @return false if the simulation stopped before the dongle was acquired.
 */
bool dongle_take(t_sim *sim, t_dongle *dongle, t_coder *coder)
{
	t_heap_node node;

	if (sim_stop_getter(sim))
		return (false);
	node = build_node(sim, coder);
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->queue, node);
	while (!sim_stop_getter(sim) && !(dongle->available && heap_top_is(&dongle->queue, coder->id)))
		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	if (sim_stop_getter(sim))
	{
		pthread_mutex_unlock(&dongle->mutex);
		return (false);
	}
	heap_pop(&dongle->queue);
	dongle->available = false;
	safe_print(sim, coder->id, "has taken a dongle");
	pthread_mutex_unlock(&dongle->mutex);
	return (true);
}

void dongle_release(t_sim *sim, t_dongle *dongle)
{
	t_timeval now;
	t_timespec timeout;

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + sim->dongle_cooldown / 1000;
	timeout.tv_nsec = (now.tv_usec * 1000) + (sim->dongle_cooldown % 1000) * 1000000;
	if (timeout.tv_nsec >= 1000000000)
	{
		timeout.tv_sec++;
		timeout.tv_nsec -= 1000000000;
	}
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = false;
	pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &timeout);
	dongle->available = true;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
