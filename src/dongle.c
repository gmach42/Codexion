/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 13:04:03 by gmach             #+#    #+#             */
/*   Updated: 2026/02/24 13:55:09 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// void	dongle_cleanup(t_dongle **dongle)
// {
// 	int	i;

// 	i = 0;
// 	while (i < dongle[i])
// 	{
// 		pthread_mutex_destroy(&dongle[i++]->mutex);
// 	}
// }

/**
 * @brief "Boolean" function to check if a dongle is takeable or not
 *
 * @param coder
 * @param dongle
 * @return 1 if the dongle can be taken by the coder,
 * 0 if the other dongle is already taken and -1 on error
 */
// int	can_take_dongle(t_coder *coder, t_dongle *dongle)
// {
// 	t_dongle	*other_dongle;

// 	if (dongle != coder->left || dongle != coder->right)
// 		printf("This dongle is neither at the left or the right of the coder");
// 		return (-1);
// 	if (dongle == coder->left)
// 		other_dongle = coder->right;
// 	else
// 		other_dongle = coder->left;
// 	if (!can_take_dongle(coder, other_dongle))
// 		printf("The other dongle is already taken");
// 		return (0);
// 	return (1);
// }

t_timespec	deadline_calculation(size_t last_compile_time, size_t time_to_burnout)
{
	t_timespec deadline;

	deadline.tv_sec = (last_compile_time + time_to_burnout) / 1000;
	deadline.tv_nsec = (last_compile_time + time_to_burnout) % 1000 * 1000000;
	return (deadline);
}

void dongle_take(t_sim *sim, t_dongle *dongle, t_coder *coder)
{
	t_timespec	deadline;

	if (!sim || !dongle || !coder)
	{
		printf("error dongle\n");
		return;
	}
	deadline = deadline_calculation(coder->last_compile_time, sim->time_to_burnout);
	pthread_mutex_lock(&dongle->mutex);
	printf("dongle locked\n");
	while (!dongle->available)
		pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &deadline); // TODO fix deadline stocker ca dans coder directement?
	pthread_mutex_unlock(&dongle->mutex);
}

void dongle_release(t_sim *sim, t_dongle *dongle)
{
	t_timeval now;
	t_timespec timeout;

	gettimeofday(&now, NULL);
	timeout.tv_sec = now.tv_sec + sim->dongle_cooldown;
	timeout.tv_nsec = (now.tv_usec * 1000) + ((sim->dongle_cooldown % 1000) * 1000000);
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = false;
	pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &timeout);
	dongle->available = true;
	pthread_cond_broadcast(&dongle->cond);
	pthread_mutex_unlock(&dongle->mutex);
}
