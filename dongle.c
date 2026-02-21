# include "codexion.h"

void	cleanup_dongle(t_dongle **dongle)
{
	int	i;

	i = 0;
	while (i < dongle[i])
	{
		pthread_mutex_destroy(&dongle[i++]->mutex);
	}
}

/**
 * @brief A coder try to take a dongle
 *
 * @param dongle
 * @param coder
 */
void	try_to_take_dongle(t_dongle *dongle, t_coder *coder)
{
	struct timeval now;
	struct timespec timeout;

	// Check if dongle is available, if not return
	if (dongle->available != 0)
		return;
	gettimeofday(&now, NULL);

	// Lock dongle
	pthread_mutex_lock(&dongle->mutex);

	while (!can_take_dongle(coder, dongle))
		pthread_cond_wait(&dongle->cond, &dongle->mutex);

	take_dongle(coder, dongle);

	// Unlock dongle
	pthread_mutex_unlock(&dongle->mutex);
}

/**
 * @brief "Boolean" function to check if a dongle is takeable or not
 *
 * @param coder
 * @param dongle
 * @return 1 if the dongle can be taken by the coder,
 * 0 if the other dongle is already taken and -1 on error
 */
int	can_take_dongle(t_coder *coder, t_dongle *dongle)
{
	t_dongle	*other_dongle;

	// Check if the coder is next to the dongle
	if (dongle != coder->left || dongle != coder->right)
		printf("This dongle is neither at the left or the right of the coder");
		return (-1);
	// Assign dongle pointer
	if (dongle == coder->left)
		other_dongle = coder->right;
	else
		other_dongle = coder->left;
	// Check if the other dongle is taken
	if (!can_take_dongle(coder, other_dongle))
		printf("The other dongle is already taken");
		return (0);
	// The dongle is takeable return 1
	return (1);
}
