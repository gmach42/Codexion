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
size_t get_current_time(void)
{
	struct timeval time;

	if (gettimeofday(&time, NULL) == -1)
		write(2, "gettimeofday() error\n", 22);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

bool is_burned(t_sim *sim)
{
	return (time_since_start(sim) > sim->time_to_burnout);
}

bool has_compiled_enough(t_sim *simulation)
{
	return (simulation->number_of_compiles >= simulation->number_of_compiles_required);
}

/**
 * @brief Check if the simulation has finished or not
 *
 * @param sim
 * @return true in case the simulation has stopped either if a coder has burned or if the simulation has compiled enough
 * @return false in case the simulaion hasn't stopped yet
 */
bool simulation_stopped(t_sim *sim)
{
	if (is_burned(sim))
	{
		printf("Coder has burnt %ld us\n", time_since_start(sim));
		return true;
	}
	if (has_compiled_enough(sim))
	{
		printf("Simulation Complete\n");
		return true;
	}
	return false;
}

void	safe_print(t_sim *sim, char *msg)
{
	pthread_mutex_lock(&sim->print_mutex);
	printf("%s", msg);
	pthread_mutex_unlock(&sim->print_mutex);
}
