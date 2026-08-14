/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:30:23 by gmach             #+#    #+#             */
/*   Updated: 2026/08/14 13:58:58 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

size_t	time_since_start(t_sim *sim)
{
	size_t	time_since_start;

	time_since_start = get_current_time() - sim->start_time;
	return (time_since_start);
}

/**
 * @brief Get the current time in millisecond
 *
 * @return size_t
 */
size_t	get_current_time(void)
{
	struct timeval	time;

	if (gettimeofday(&time, NULL) == -1)
		write(2, "gettimeofday() error\n", 22);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

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
