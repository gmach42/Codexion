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

static bool	take_dongles(t_coder *coder)
{
	size_t	arrival;
	bool	first_round;

	pthread_mutex_lock(&coder->time_mutex);
	first_round = (coder->compile_count == 0);
	pthread_mutex_unlock(&coder->time_mutex);
	if (first_round && coder->id % 2 == 0)
		better_sleep(coder->sim, 5);
	arrival = get_current_time();
	enqueue_dongles(coder, arrival);
	if (coder->id % 2 == 1)
	{
		if (!dongle_wait_and_take(coder->sim, coder->left, coder))
			return (false);
		if (!dongle_wait_and_take(coder->sim, coder->right, coder))
			return (false);
	}
	else
	{
		if (!dongle_wait_and_take(coder->sim, coder->right, coder))
			return (false);
		if (!dongle_wait_and_take(coder->sim, coder->left, coder))
			return (false);
	}
	return (true);
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
