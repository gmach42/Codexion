#include "codexion.h"

int main(int argc, char **argv)
{
	t_simulation	sim;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_t		monitor_thread;
	int				i;

	if (argc != 9)
	{
		fprintf(stderr, "Expecting 8 args received %d args\n", argc - 1);
		fprintf(stderr, "number_of_coders, time_to_burnout, time_to_compile, time_to_debug, time_to_refactor, number_of_compiles_required, dongle_cooldown, scheduler");
		return (0);
	}

	sim = simulation_init(argv);
	printf("Simulation init!\n");
	dongles_init(&sim);
	printf("Dongles init!\n");
	coders_init(&sim);
	printf("Coders init!\n");
	// monitor_thread = monitor_init(sim);

	while (!simulation_stopped(&sim))
		{
			i = 0;
			while(i < sim.number_of_coders)
				{
					printf("trying routine coder %d\n", i);
					routine(&sim, &sim.coders[i++]);
				}
		}

	//destroy(simulation, coders, dongles, monitor_thread);

    return 0;
}
