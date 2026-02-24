/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:03:55 by gmach             #+#    #+#             */
/*   Updated: 2026/02/24 13:50:04 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_sim simulation_init(char **argv)
{
	t_sim sim;

	sim.number_of_coders = atoi(argv[1]);
	sim.time_to_burnout = atoi(argv[2]) * 1000000;
	sim.time_to_compile = atoi(argv[3]) * 1000000;
	sim.time_to_debug = atoi(argv[4]) * 1000000;
	sim.time_to_refactor = atoi(argv[5]) * 1000000;
	sim.number_of_compiles_required = atoi(argv[6]);
	sim.dongle_cooldown = atoi(argv[7]) * 1000000;
	sim.start_time = get_current_time();
	sim.stop = false;

	coders_init(&sim);
	dongles_init(&sim);

	return (sim);
}

void coder_init(t_coder *coder, int id, t_dongle *left, t_dongle *right)
{
	coder->id = id;
	coder->left = left;
	coder->right = right;
	coder->state = INIT;
	coder->last_compile_time = get_current_time();
}

void coders_init(t_sim *sim)
{
	t_coder *coders;
	t_dongle *dongles;
	int i;

	i = 0;
	coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!coders)
		return;
	while (i < sim->number_of_coders)
	{
		coder_init(&coders[i], i, &sim->dongles[i], &sim->dongles[(i + 1) % sim->number_of_coders]);
		coders[i++].sim = sim;
	}
	sim->coders = coders;
}

void dongle_init(t_dongle *dongle)
{
	cond_t cond;
	mutex_t mutex;
	t_heapq *waiting_q;

	waiting_q = malloc(sizeof(t_heapq) * 2);
	if (!waiting_q)
	{
		printf("Error while creating dongles\n");
		exit(EXIT_FAILURE);
	}
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
	dongle->available = true;
	dongle->cond = cond;
	dongle->mutex = mutex;
	printf("mutex %X\n",mutex);
	dongle->schedule = waiting_q;
}

void dongles_init(t_sim *sim)
{
	t_dongle *dongles;
	int i;

	i = 0;
	dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if (!dongles)
	{
		printf("Error while creating dongles\n");
		exit(EXIT_FAILURE);
	}
	while (i < sim->number_of_coders)
	{
		dongle_init(&dongles[i]);
		i++;
	}
	sim->dongles = dongles;
}
