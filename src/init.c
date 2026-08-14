/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:30:13 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 15:45:01 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	simulation_init(t_sim *sim, char **argv)
{
	sim->nb_coders = parse_int(argv[1]);
	if (sim->nb_coders < 1)
	{
		fprintf(stderr, "Invalid argument: number_of_coders must be >= 1\n");
		exit(EXIT_FAILURE);
	}
	sim->time_to_burnout = parse_int(argv[2]);
	sim->time_to_compile = parse_int(argv[3]);
	sim->time_to_debug = parse_int(argv[4]);
	sim->time_to_refactor = parse_int(argv[5]);
	sim->nb_compiles = parse_int(argv[6]);
	sim->dongle_cd = parse_int(argv[7]);
	sim->dongle_schedule = parse_str(argv[8]);
	sim->start_time = get_current_time();
	sim->stop = false;
	pthread_mutex_init(&sim->stop_mutex, NULL);
	pthread_mutex_init(&sim->print_mutex, NULL);
}

void	coder_init(t_sim *sim, t_coder *coder, int id, t_dongle *dongles)
{
	coder->id = id;
	coder->left = &dongles[id - 1];
	coder->right = &dongles[id % sim->nb_coders];
	coder->sim = sim;
	coder->last_compile_time = sim->start_time;
	coder->compile_count = 0;
	pthread_mutex_init(&coder->time_mutex, NULL);
}

bool	coders_init(t_sim *sim)
{
	t_coder		*coders;
	int			i;

	i = 0;
	coders = malloc(sizeof(t_coder) * sim->nb_coders);
	if (!coders)
		return (false);
	while (i < sim->nb_coders)
	{
		coder_init(sim, &coders[i], i + 1, sim->dongles);
		coders[i++].sim = sim;
	}
	sim->coders = coders;
	return (true);
}

bool	dongle_init(t_dongle *dongle, int capacity)
{
	pthread_cond_init(&dongle->cond, NULL);
	pthread_mutex_init(&dongle->mutex, NULL);
	dongle->available = true;
	dongle->cooldown = 0;
	dongle->queue.nodes = malloc(sizeof(t_hnode) * capacity);
	if (!dongle->queue.nodes)
		return (false);
	dongle->queue.size = 0;
	dongle->queue.capacity = capacity;
	return (true);
}

bool	dongles_init(t_sim *sim)
{
	t_dongle	*dongles;
	int			i;

	i = 0;
	dongles = malloc(sizeof(t_dongle) * sim->nb_coders);
	if (!dongles)
		return (false);
	sim->dongles = dongles;
	while (i < sim->nb_coders)
	{
		if (!dongle_init(&dongles[i], sim->nb_coders))
		{
			sim->nb_coders = i;
			free_dongles(sim);
			return (false);
		}
		i++;
	}
	return (true);
}
