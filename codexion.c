#include "codexion.h"

int main(int argc, char **argv)
{
	t_simulation	simulation;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor_thread;

	if (argc != 9)
	{
		printf("Expecting 8 args received %d args", argc - 1);
		return (0);
	}

	simulation = simulation_init(argv);
	dongles = dongles_init(simulation);
	coders = coders_init(simulation);
	monitor_thread = monitor_init(simulation);

	/**
	 * @brief Each coders thread routine will try to take the left and right dongle, compile, debug and refactor until burnout
	 * The routine will end when the coder has compiled the required number of times or is burned
	 */
	while (monitor_thread)
		routine(simulation, coders);

	destroy(simulation, coders, dongles, monitor_thread);

    return 0;
}

void routine(t_simulation simulation, t_coder *coder)
{
	while (!is_burned(simulation) || has_compiled_enough(simulation))
	{
		try_to_take_dongles(simulation, coder);
		compile(simulation, coder);
		debug(simulation, coder);
		refactor(simulation, coder);
	}
}

// Check if the coder has been burned out based on the time since last compile and the burnout threshold
bool is_burned(t_simulation simulation)
{
	struct timeval now;

	gettimeofday(&now, NULL);
    double time_passed;
    time_passed = (now.tv_sec - simulation.start_time.tv_sec) * 1e6;
    time_passed = (time_passed + (now.tv_usec - simulation.start_time.tv_usec)) * 1e-6;
	return (time_passed > simulation.time_to_burnout)
}

bool has_compiled_enough(t_simulation simulation)
{
	return (simulation.number_of_compiles >= simulation.number_of_compiles_required);
}

try_to_take_dongles(t_simulation simulation, t_coder *coder)
{
	// Attempt to take the left and right dongle, waiting if necessary until they are available
}

compile(t_simulation simulation, t_coder *coder)
{
	// Simulate the compile action, update coder's state and release dongles after use
}

debug(t_simulation simulation, t_coder *coder)
{
	// Simulate the debug action, update coder's state
}

refactor(t_simulation simulation, t_coder *coder)
{
	// Simulate the refactor action, update coder's state
}
