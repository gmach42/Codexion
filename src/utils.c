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

bool	is_burned(t_sim *sim)
{
	return (time_since_start(sim) > sim->time_to_burnout);
}

bool	has_compiled_enough(t_sim *sim)
{
	return (sim->number_of_compiles >= sim->number_of_compiles_required);
}
