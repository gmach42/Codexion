/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:29:52 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 17:38:05 by gmach            ###   ########lyon.fr   */
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
	free_dongles(sim);
	free(sim->coders);
}

static bool	check_args(int argc)
{
	if (argc != 9)
	{
		fprintf(stderr, "Expecting 8 args received %d args\n", argc - 1);
		fprintf(stderr,
			"number_of_coders, time_to_burnout, time_to_compile, "
			"time_to_debug, time_to_refactor, number_of_compiles_required, "
			"dongle_cd, scheduler\n");
		return (false);
	}
	return (true);
}

int	main(int argc, char **argv)
{
	t_sim		sim;
	pthread_t	monitor_thread;

	if (!check_args(argc))
		return (1);
	simulation_init(&sim, argv);
	if (!dongles_init(&sim))
		return (1);
	if (!coders_init(&sim))
	{
		free_dongles(&sim);
		return (1);
	}
	sim.start_time = get_current_time();
	pthread_create(&monitor_thread, NULL, monitor_routine, &sim);
	launch_coders(&sim);
	join_coders(&sim, monitor_thread);
	clean_up(&sim);
	return (0);
}
