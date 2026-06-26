/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:29:52 by gmach             #+#    #+#             */
/*   Updated: 2026/06/26 14:16:32 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// void print_sim_state(t_sim *sim)
// {
// 	printf("\nSIM STATE\n");
// 	printf("coders: %X\n", sim->coders);
// 	printf("dongles: %X\n", sim->dongles);
// 	printf("number of coders: %d\n", sim->number_of_coders);
// 	printf("compiles done: %d\n", sim->number_of_compiles);
// 	printf("compiles to do: %d\n", sim->number_of_compiles_required);
// 	printf("time to burnout: %.1f s\n", (float)sim->time_to_burnout / 1000000);
// 	printf("time to debug: %.1f s\n", (float)sim->time_to_debug / 1000000);
// 	printf("time to compile: %.1f s\n", (float)sim->time_to_compile / 1000000);
// 	printf("time to refactor: %.1f s\n", (float)sim->time_to_refactor / 1000000);
// 	printf("dongle cooldown: %.1f s\n", (float)sim->dongle_cooldown / 1000000);
// 	printf("start time: %d\n\n", sim->start_time);
// }

bool sim_stop_getter(t_sim *sim)
{
	bool stopped;

	pthread_mutex_lock(&sim->stop_mutex);
	stopped = sim->stop;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (stopped);
}

void safe_print(t_sim *sim, int id, char *msg)
{
	pthread_mutex_lock(&sim->print_mutex);
	if (!sim_stop_getter(sim))
		printf("%zu %d %s\n",
			   get_current_time() - sim->start_time,
			   id,
			   msg);
	pthread_mutex_unlock(&sim->print_mutex);
}

void sim_stop_setter(t_sim *sim)
{
	int i;

	pthread_mutex_lock(&sim->stop_mutex);
	sim->stop = true;
	pthread_mutex_unlock(&sim->stop_mutex);
	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->dongles[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

void clean_up(t_sim *sim)
{
	int i;

	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->print_mutex);
	i = 0;
	while (i < sim->number_of_coders)
		pthread_mutex_destroy(&sim->coders[i++].time_mutex);
	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_cond_destroy(&sim->dongles[i].cond);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		free(sim->dongles[i].queue.nodes);
		i++;
	}
	free(sim->coders);
	free(sim->dongles);
}

int main(int argc, char **argv)
{
	t_sim sim;
	pthread_t monitor_thread;
	int i;

	if (argc != 9)
	{
		fprintf(stderr, "Expecting 8 args received %d args\n", argc - 1);
		fprintf(stderr, "number_of_coders, time_to_burnout, time_to_compile, time_to_debug, time_to_refactor, number_of_compiles_required, dongle_cooldown, scheduler");
		return (1);
	}
	simulation_init(&sim, argv);
	dongles_init(&sim);
	coders_init(&sim);
	sim.start_time = get_current_time();
	pthread_create(&monitor_thread, NULL, monitor_routine, &sim);
	i = 0;
	while (i < sim.number_of_coders)
	{
		if (i % 2 == 0)
			usleep(1000);
		pthread_create(&sim.coders[i].thread, NULL, routine, &sim.coders[i]);
		i++;
	}
	i = 0;
	pthread_join(monitor_thread, NULL);
	while (i < sim.number_of_coders)
		pthread_join(sim.coders[i++].thread, NULL);
	clean_up(&sim);
	return (0);
}
