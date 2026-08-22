/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:29:58 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 19:51:10 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	compile(t_sim *sim, t_coder *coder)
{
	if (sim_stop_getter(sim))
		return (false);
	safe_print(sim, coder->id, "is compiling");
	pthread_mutex_lock(&coder->time_mutex);
	coder->last_compile_time = get_current_time();
	pthread_mutex_unlock(&coder->time_mutex);
	better_sleep(sim, sim->time_to_compile);
	dongle_release(sim, coder->left);
	if (coder->right != coder->left)
		dongle_release(sim, coder->right);
	return (true);
}

bool	debug(t_sim *sim, t_coder *coder)
{
	if (sim_stop_getter(sim))
		return (false);
	safe_print(sim, coder->id, "is debugging");
	better_sleep(sim, sim->time_to_debug);
	return (true);
}

bool	refactor(t_sim *sim, t_coder *coder)
{
	if (sim_stop_getter(sim))
		return (false);
	safe_print(sim, coder->id, "is refactoring");
	better_sleep(sim, sim->time_to_refactor);
	return (true);
}

/**
 * @brief First-round-only stagger: without it, every coder's initial
 *        deadline is identical (last_compile_time == sim start_time for
 *        everyone), so odd/even neighbors would tie and only the lowest
 *        coder id could ever win any contested dongle, serializing the
 *        whole ring instead of letting alternating pairs compile in
 *        parallel. This is dropped after round 1: from then on, each
 *        coder's own compile history naturally spreads deadlines apart.
 */
static bool	take_dongles(t_coder *coder)
{
	bool	first_round;

	pthread_mutex_lock(&coder->time_mutex);
	first_round = (coder->compile_count == 0);
	pthread_mutex_unlock(&coder->time_mutex);
	if (first_round && coder->id % 2 == 0)
		better_sleep(coder->sim, 5);
	dongles_request(coder->sim, coder, get_current_time());
	return (dongles_acquire(coder->sim, coder));
}

void	*routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!sim_stop_getter(coder->sim))
	{
		if (!take_dongles(coder))
			break ;
		if (!compile(coder->sim, coder))
			break ;
		if (!debug(coder->sim, coder))
			break ;
		if (!refactor(coder->sim, coder))
			break ;
		pthread_mutex_lock(&coder->time_mutex);
		coder->compile_count++;
		pthread_mutex_unlock(&coder->time_mutex);
		if (sim_stop_getter(coder->sim))
			break ;
	}
	return (NULL);
}
