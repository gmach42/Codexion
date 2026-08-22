/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:04:03 by gmach             #+#    #+#             */
/*   Updated: 2026/08/22 00:00:00 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Build a heap node for `coder`'s current request.
 *        FIFO: priority = arrival time (ms).
 *        EDF : priority = last_compile_start + time_to_burnout (ms),
 *              tiebreak = arrival time.
 */
static t_hnode	build_node(t_sim *sim, t_coder *coder, size_t arrival)
{
	t_hnode	node;

	node.coder_id = coder->id;
	node.tiebreak = arrival;
	node.pending = true;
	if (sim->dongle_schedule == FIFO)
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
 * @brief Mark `coder` as actively waiting on both of its dongles at
 *        once. First call ever for a given dongle inserts the node
 *        (heap_upsert); every later call just refreshes its priority
 *        and flips it back to pending, in place. Since both dongles
 *        are updated together, a coder can never be visible in only
 *        one of its two queues.
 */
void	dongles_request(t_sim *sim, t_coder *coder, size_t arrival)
{
	t_hnode	node;

	node = build_node(sim, coder, arrival);
	pthread_mutex_lock(&sim->dongle_mutex);
	heap_upsert(&coder->left->queue, node);
	if (coder->right != coder->left)
		heap_upsert(&coder->right->queue, node);
	pthread_mutex_unlock(&sim->dongle_mutex);
}

static bool	dongle_ready(t_dongle *dongle, int coder_id)
{
	return (dongle->available && get_current_time() >= dongle->cooldown
		&& heap_pending_top_is(&dongle->queue, coder_id));
}

/**
 * @brief A coder needs 2 distinct dongles to compile. With a single
 *        coder, left and right are the same physical dongle, so it can
 *        never actually gather 2 - it must wait forever (until burnout
 *        stops the sim), matching "if only one coder is present, only
 *        one dongle will be available".
 */
static bool	both_ready(t_coder *coder)
{
	if (coder->left == coder->right)
		return (false);
	return (dongle_ready(coder->left, coder->id)
		&& dongle_ready(coder->right, coder->id));
}

static void	dongles_wait(t_sim *sim, t_coder *coder)
{
	t_timespec	timeout;
	size_t		wake;

	while (!sim_stop_getter(sim) && !both_ready(coder))
	{
		wake = 0;
		if (coder->left->available
			&& get_current_time() < coder->left->cooldown)
			wake = coder->left->cooldown;
		if (coder->right != coder->left && coder->right->available
			&& get_current_time() < coder->right->cooldown
			&& (wake == 0 || coder->right->cooldown < wake))
			wake = coder->right->cooldown;
		if (wake)
		{
			timeout.tv_sec = wake / 1000;
			timeout.tv_nsec = (wake % 1000) * 1000000;
			pthread_cond_timedwait(&coder->cond, &sim->dongle_mutex,
				&timeout);
		}
		else
			pthread_cond_wait(&coder->cond, &sim->dongle_mutex);
	}
}

/**
 * @brief Block until `coder` is granted both of its dongles (already
 *        marked pending via a prior dongles_request), then take them
 *        atomically: never holding one while still waiting on the
 *        other. The node itself is kept in the queue (not removed),
 *        just flipped back to non-pending until the next round.
 */
bool	dongles_acquire(t_sim *sim, t_coder *coder)
{
	pthread_mutex_lock(&sim->dongle_mutex);
	dongles_wait(sim, coder);
	if (sim_stop_getter(sim))
	{
		pthread_mutex_unlock(&sim->dongle_mutex);
		return (false);
	}
	heap_set_pending(&coder->left->queue, coder->id, false);
	coder->left->available = false;
	safe_print(sim, coder->id, "has taken a dongle");
	if (coder->right != coder->left)
	{
		heap_set_pending(&coder->right->queue, coder->id, false);
		coder->right->available = false;
		safe_print(sim, coder->id, "has taken a dongle");
	}
	pthread_mutex_unlock(&sim->dongle_mutex);
	return (true);
}

/**
 * @brief Wake only the (at most 2) coders who actually have a stake in
 *        this dongle, instead of every coder in the simulation. With a
 *        single shared condvar, every release would broadcast to all N
 *        threads regardless of relevance - fine at N=11, but at N=100+
 *        that thundering herd (all N threads waking, fighting over the
 *        lock, going back to sleep) adds real scheduling overhead that
 *        eats into tight burnout budgets.
 */
static void	dongle_wake_waiters(t_sim *sim, t_dongle *dongle)
{
	int	i;

	i = 0;
	while (i < dongle->queue.size)
	{
		pthread_cond_signal(&sim->coders[dongle->queue.nodes[i].coder_id
			- 1].cond);
		i++;
	}
}

void	dongle_release(t_sim *sim, t_dongle *dongle)
{
	pthread_mutex_lock(&sim->dongle_mutex);
	dongle->available = true;
	dongle->cooldown = get_current_time() + sim->dongle_cd;
	dongle_wake_waiters(sim, dongle);
	pthread_mutex_unlock(&sim->dongle_mutex);
}

void	free_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
		free(sim->dongles[i++].queue.nodes);
	free(sim->dongles);
}
