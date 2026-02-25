/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:04:03 by gmach             #+#    #+#             */
/*   Updated: 2026/02/25 10:25:58 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_timespec	deadline_calculation(size_t last_compile_time, size_t time_to_burnout)
{
	t_timespec	deadline;
	size_t		deadline_ms;

	deadline_ms = last_compile_time + time_to_burnout;
	deadline.tv_sec = deadline_ms / 1000;
	deadline.tv_nsec = (deadline_ms % 1000) * 1000000;
	return (deadline);
}

void	dongle_take(t_sim *sim, t_dongle *dongle, t_coder *coder)
{
	t_timespec	deadline;

	deadline = deadline_calculation(coder->last_compile_time, sim->time_to_burnout);
	pthread_mutex_lock(&dongle->mutex);
	while (!dongle->available)
	{
		if (sim->dongle_schedule == 0)
			pthread_cond_wait(&dongle->cond, &dongle->mutex);
		else
			pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &deadline);
	}
	dongle->available = false;
	safe_print(sim, coder->id, "has taken a dongle");
	pthread_mutex_unlock(&dongle->mutex);
}

void	dongle_release(t_sim *sim, t_dongle *dongle)
{
	t_timeval	now;
	t_timespec	timeout;

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + sim->dongle_cooldown / 1000;
	timeout.tv_nsec = (now.tv_usec * 1000) + (sim->dongle_cooldown % 1000) * 1000000;
	if (timeout.tv_nsec >= 1000000000)
	{
		timeout.tv_sec++;
		timeout.tv_nsec -= 1000000000;
	}
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = false;
	pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &timeout);
	dongle->available = true;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
