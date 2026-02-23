/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:03:55 by gmach             #+#    #+#             */
/*   Updated: 2026/02/23 16:14:50 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_simulation	simulation_init(char **argv)
{
	t_simulation	simulation;
	t_timeval		now;

	gettimeofday(&now, NULL);
	simulation.number_of_coders = atoi(argv[1]);
	simulation.time_to_burnout = atoi(argv[2]);
	simulation.time_to_compile = atoi(argv[3]);
	simulation.time_to_debug = atoi(argv[4]);
	simulation.time_to_refactor = atoi(argv[5]);
	simulation.number_of_compiles_required = atoi(argv[6]);
	simulation.dongle_cooldown = atoi(argv[7]);
	simulation.start_time = now.tv_sec;

	return (simulation);
}

t_coder	coder_init(t_coder *coder, int id, t_dongle *left, t_dongle *right)
{
	coder->id = id;
	coder->left = left;
	coder->right = right;
	coder->state = INIT;
	printf("Coder %d initialize, left dongle: %X, right dongle %X\n", id, left, right);
}

void	*coders_init(t_simulation *sim)
{
	t_coder		*coders;
	t_dongle	*dongles;
	int			i;

	i = 0;
	coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!coders)
		return (NULL);
	while (i < sim->number_of_coders)
	{
		coder_init(&coders[i], i, &sim->dongles[i], &sim->dongles[(i + 1) % sim->number_of_coders]);
		i++;
	}
	sim->coders = coders;
}

void	dongle_init(t_dongle *dongle)
{
	pthread_cond_t	cond;
	pthread_mutex_t	mutex;
	t_heapq			*waiting_q;

	waiting_q = malloc(sizeof(t_heapq) * 2);
	if (!waiting_q)
		return;
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
	dongle->available = 1;
	dongle->cond = cond;
	dongle->mutex = mutex;
	dongle->schedule = waiting_q;
}

void	*dongles_init(t_simulation *sim)
{
	t_dongle	*dongles;
	int			i;

	i = 0;
	dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if (!dongles)
		return (NULL);
	while (i < sim->number_of_coders)
	{
		dongle_init(&dongles[i]);
		i++;
	}
	sim->dongles = dongles;
}

