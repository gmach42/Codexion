/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:29:52 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 14:03:20 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	sim_stop_getter(t_sim *sim)
{
	bool	stopped;

	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stopped);
}

void	safe_print(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->print_mutex);
	if (!sim_stop_getter(sim))
		printf("%zu %d %s\n",
			get_current_time() - sim->start_time, id, msg);
	pthread_mutex_unlock(&sim->print_mutex);
}

void	clean_up(t_sim *sim)
{
	int	i;

	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->print_mutex);
	i = 0;
	while (i < sim->nb_coders)
		pthread_mutex_destroy(&sim->coders[i++].time_mutex);
	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_cond_destroy(&sim->dongles[i].cond);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		free(sim->dongles[i].queue.nodes);
		i++;
	}
	free(sim->coders);
	free(sim->dongles);
}

static void	launch_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		if (i % 2 == 0)
			usleep(1000);
		pthread_create(&sim->coders[i].thread, NULL, routine, &sim->coders[i]);
		i++;
	}
}

int	main(int argc, char **argv)
{
	t_sim		sim;
	pthread_t	monitor_thread;
	int			i;

	if (argc != 9)
	{
		fprintf(stderr, "Expecting 8 args received %d args\n", argc - 1);
		fprintf(stderr,
			"number_of_coders, time_to_burnout, time_to_compile, "
			"time_to_debug, time_to_refactor, number_of_compiles_required, "
			"dongle_cd, scheduler\n");
		return (1);
	}
	simulation_init(&sim, argv);
	dongles_init(&sim);
	// if alloc failed, free simulation alloc then exit
	coders_init(&sim);
	// if alloc failed, free simulation and dongle then exit
	sim.start_time = get_current_time();
	pthread_create(&monitor_thread, NULL, monitor_routine, &sim);
	launch_coders(&sim);
	i = 0;
	pthread_join(monitor_thread, NULL);
	while (i < sim.nb_coders)
		pthread_join(sim.coders[i++].thread, NULL);
	clean_up(&sim);
	return (0);
}
