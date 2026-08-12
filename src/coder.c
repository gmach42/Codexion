/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:29:58 by gmach             #+#    #+#             */
/*   Updated: 2026/02/25 17:29:59 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	better_sleep(t_sim *sim, size_t duration_ms)
{
	size_t	start_time;

	start_time = get_current_time();
	while (!sim_stop_getter(sim))
	{
		if (get_current_time() - start_time >= duration_ms)
			break ;
		usleep(duration_ms * 1000 / 10);
	}
}

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

void	*routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	while (!sim_stop_getter(coder->sim))
	{
		/* Always request dongles in a fixed global order (lowest memory
		 * address first). This total ordering breaks the circular wait
		 * (Coffman condition) that would otherwise deadlock the ring
		 * if every coder grabbed left then right in lock-step. */
		if (coder->left < coder->right)
		{
			if (!dongle_take(coder->sim, coder->left, coder))
				break ;
			if (!dongle_take(coder->sim, coder->right, coder))
				break ;
		}
		else
		{
			if (!dongle_take(coder->sim, coder->right, coder))
				break ;
			if (!dongle_take(coder->sim, coder->left, coder))
				break ;
		}
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
