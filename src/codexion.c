#include "codexion.h"

void	print_sim_state(t_simulation *sim)
{
	printf("\nSIM STATE\n");
	printf("coders: %X\n", sim->coders);
	printf("dongles: %X\n", sim->dongles);
	printf("number of coders: %d\n", sim->number_of_coders);
	printf("compiles done: %d\n", sim->number_of_compiles);
	printf("compiles to do: %d\n", sim->number_of_compiles_required);
	printf("time to burnout: %d\n", sim->time_to_burnout);
	printf("time to debug: %d\n", sim->time_to_debug);
	printf("time to compile: %d\n", sim->time_to_compile);
	printf("time to refactor: %d\n", sim->time_to_refactor);
	printf("dongle cooldown: %d\n", sim->dongle_cooldown);
	printf("start time: %d\n", sim->start_time);
}

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

	printf("\n	INIT:\n\n");

	sim = simulation_init(argv);
	printf("Simulation init!\n");
	dongles_init(&sim);
	printf("Dongles init!\n");
	coders_init(&sim);
	printf("Coders init!\n");
	// monitor_thread = monitor_init(sim);

	print_sim_state(&sim);

	printf("\n	START:\n\n");
	while (!simulation_stopped(&sim))
		{
			i = 0;
			while(i < sim.number_of_coders)
				{
					printf("trying routine coder %d\n", i);
					routine(&sim, &sim.coders[i++]);
					print_sim_state(&sim);
				}
		}

	//destroy(simulation, coders, dongles, monitor_thread);

    return 0;
}


