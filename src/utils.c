#include "codexion.h"

t_timeval	time_since_start(t_simulation *sim)
{
	t_timeval	current_time;
	t_timeval	time_since_start;

	gettimeofday(&current_time, NULL);
	time_since_start.tv_sec = current_time.tv_sec - sim->start_time;
	time_since_start.tv_usec = current_time.tv_usec - sim->start_time * 1000;
	return (time_since_start);
}

bool is_burned(t_simulation *sim)
{
	return (time_since_start(sim).tv_sec > sim->time_to_burnout);
}

bool has_compiled_enough(t_simulation *simulation)
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
bool	simulation_stopped(t_simulation *sim)
{
	if (is_burned(sim))
	{
		printf("Coder has burnt %d", sim->start_time);
		return true;
	}
	if (has_compiled_enough(sim))
	{
		printf("Simulation Complete");
		return true;
	}
	return false;
}
