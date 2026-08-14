/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:04:03 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 17:14:16 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Build a heap node for `coder` requesting a `dongle`.
 *        FIFO: priority = arrival time (ms).
 *        EDF : priority = last_compile_start + time_to_burnout (ms),
 *              tiebreak = arrival time.
 */
static t_hnode	build_node(t_sim *sim, t_coder *coder, size_t arrival)
{
	t_hnode	node;

	node.coder_id = coder->id;
	node.tiebreak = arrival;
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

static void	dongle_wait(t_sim *sim, t_dongle *dongle, t_coder *coder)
{
	t_timespec	timeout;

	while (!sim_stop_getter(sim)
		&& !(dongle->available && heap_top_is(&dongle->queue, coder->id)))
	{
		if (dongle->available && get_current_time() < dongle->cooldown)
		{
			timeout.tv_sec = dongle->cooldown / 1000;
			timeout.tv_nsec = (dongle->cooldown % 1000) * 1000000;
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &timeout);
		}
		else
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}
}

/**
 * @brief Try to acquire `dongle` for a `coder`.
 *        Only takes the dongle when:
 *          - it is at the head of the priority queue, AND
 *          - the dongle is not in its cooldown period.
 *
 * @return true  on success (dongle taken).
 * @return false if the simulation stopped before the dongle was acquired.
 */
bool	dongle_take(t_sim *sim, t_dongle *dongle, t_coder *coder, size_t arrival)
{
	t_hnode	node;

	if (sim_stop_getter(sim))
		return (false);
	node = build_node(sim, coder, arrival);
	pthread_mutex_lock(&dongle->mutex);
	heap_push(&dongle->queue, node);
	dongle_wait(sim, dongle, coder);
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

void	dongle_release(t_sim *sim, t_dongle *dongle)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = true;
	dongle->cooldown = get_current_time() + sim->dongle_cd;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}

void	free_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_cond_destroy(&sim->dongles[i].cond);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		free(sim->dongles[i].queue.nodes);
		i++;
	}
	free(sim->dongles);
}
