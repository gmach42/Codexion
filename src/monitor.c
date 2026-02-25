/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:30:30 by gmach             #+#    #+#             */
/*   Updated: 2026/02/25 17:55:19 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"


bool	burnout(t_sim *sim, t_coder *coder, size_t last_compile_time)
{
	if (sim_stop_getter(sim))
		return (false);
	if (get_current_time() - last_compile_time > sim->time_to_burnout)
	{
		safe_print(sim, coder->id, "burned out");
		sim_stop_setter(sim);
		return (true);
	}
	return (false);
}

bool	complete(t_sim *sim)
{
	if (sim_stop_getter(sim))
		return (false);
	if (sim->number_of_compiles >= sim->number_of_compiles_required)
	{
		printf("%zu Simulation Complete\n", get_current_time() - sim->start_time);
		sim_stop_setter(sim);
		return (true);
	}
	return (false);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;
	size_t	last;

	sim = (t_sim *)arg;
	while (!sim_stop_getter(sim))
	{
		i = 0;
		while (i < sim->number_of_coders)
		{
			pthread_mutex_lock(&sim->coders[i].time_mutex);
			last = sim->coders[i].last_compile_time;
			pthread_mutex_unlock(&sim->coders[i].time_mutex);
			if (burnout(sim, &sim->coders[i], last))
				return (NULL);
			if (complete(sim))
				return (NULL);
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
