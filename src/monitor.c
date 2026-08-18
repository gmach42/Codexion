/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:30:30 by gmach             #+#    #+#             */
/*   Updated: 2026/08/18 15:24:23 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	sim_stop_setter(t_sim *sim)
{
	int	i;

	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = true;
	pthread_mutex_unlock(&sim->stop_mutex);
	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

static bool	burnout(t_sim *sim, t_coder *coder, size_t last_compile_time)
{
	if (sim_stop_getter(sim))
		return (false);
	if (get_current_time() - last_compile_time > sim->time_to_burnout)
	{
		pthread_mutex_lock(&sim->print_mutex);
		pthread_mutex_lock(&sim->stop_mutex);
		sim->stop = true;
		pthread_mutex_unlock(&sim->stop_mutex);
		printf("%zu %d burned out\n",
			get_current_time() - sim->start_time, coder->id);
		pthread_mutex_unlock(&sim->print_mutex);
		sim_stop_setter(sim);
		return (true);
	}
	return (false);
}

static bool	complete(t_sim *sim)
{
	int	i;
	int	count;

	if (sim_stop_getter(sim))
		return (false);
	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_mutex_lock(&sim->coders[i].time_mutex);
		count = sim->coders[i].compile_count;
		pthread_mutex_unlock(&sim->coders[i].time_mutex);
		if (count < sim->nb_compiles)
			return (false);
		i++;
	}
	sim_stop_setter(sim);
	return (true);
}
// printf("%zu Simulation Complete\n", get_current_time() - sim->start_time);

void	*monitor_routine(void *arg)
{
	t_sim	*sim;
	int		i;
	size_t	last;

	sim = (t_sim *)arg;
	while (!sim_stop_getter(sim))
	{
		if (complete(sim))
			return (NULL);
		i = 0;
		while (i < sim->nb_coders)
		{
			pthread_mutex_lock(&sim->coders[i].time_mutex);
			last = sim->coders[i].last_compile_time;
			pthread_mutex_unlock(&sim->coders[i].time_mutex);
			if (burnout(sim, &sim->coders[i], last))
				return (NULL);
			i++;
		}
		usleep(1000);
	}
	return (NULL);
}
