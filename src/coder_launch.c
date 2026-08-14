/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_launch.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/14 14:30:00 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 14:23:58 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	launch_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->nb_coders)
	{
		pthread_create(&sim->coders[i].thread, NULL, routine, &sim->coders[i]);
		i++;
	}
}

void	join_coders(t_sim *sim, pthread_t monitor_thread)
{
	int	i;

	i = 0;
	pthread_join(monitor_thread, NULL);
	while (i < sim->nb_coders)
		pthread_join(sim->coders[i++].thread, NULL);
}
