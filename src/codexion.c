#include "codexion.h"

// void print_sim_state(t_sim *sim)
// {
// 	printf("\nSIM STATE\n");
// 	printf("coders: %X\n", sim->coders);
// 	printf("dongles: %X\n", sim->dongles);
// 	printf("number of coders: %d\n", sim->number_of_coders);
// 	printf("compiles done: %d\n", sim->number_of_compiles);
// 	printf("compiles to do: %d\n", sim->number_of_compiles_required);
// 	printf("time to burnout: %.1f s\n", (float)sim->time_to_burnout / 1000000);
// 	printf("time to debug: %.1f s\n", (float)sim->time_to_debug / 1000000);
// 	printf("time to compile: %.1f s\n", (float)sim->time_to_compile / 1000000);
// 	printf("time to refactor: %.1f s\n", (float)sim->time_to_refactor / 1000000);
// 	printf("dongle cooldown: %.1f s\n", (float)sim->dongle_cooldown / 1000000);
// 	printf("start time: %d\n\n", sim->start_time);
// }

// int sim_stopped(t_sim *sim)
// {
//     int stopped;

//     pthread_mutex_lock(&sim->stop_mutex);
//     stopped = sim->stop;
//     pthread_mutex_unlock(&sim->stop_mutex);
//     return (stopped);
// }

void stop_simulation(t_sim *sim)
{
    pthread_mutex_lock(&sim->stop_mutex);
    sim->stop = true;
    pthread_mutex_unlock(&sim->stop_mutex);
}

void *monitor_routine(void *arg)
{
    t_sim *sim = (t_sim *)arg;
    int i;

    while (!simulation_stopped(sim))
    {
        i = 0;
        while (i < sim->number_of_coders)
        {
            if (get_current_time() - sim->coders[i].last_compile_time > sim->time_to_burnout)
            {
                printf("%d has burned out\n", sim->coders[i].id);
                stop_simulation(sim);
                return (NULL);
            }
            i++;
        }
        usleep(1000);
    }
    return (NULL);
}

int main(int argc, char **argv)
{
	t_sim sim;
	t_coder *coders;
	t_dongle *dongles;
	pthread_t monitor_thread;
	int i;

	if (argc != 9)
	{
		fprintf(stderr, "Expecting 8 args received %d args\n", argc - 1);
		fprintf(stderr, "number_of_coders, time_to_burnout, time_to_compile, time_to_debug, time_to_refactor, number_of_compiles_required, dongle_cooldown, scheduler");
		return (0);
	}

	sim = simulation_init(argv);
	pthread_create(&monitor_thread, NULL, monitor_routine, &sim);

	printf("\n	START:\n\n");
	i = 0;
	while (i < sim.number_of_coders)
	{
		pthread_create(&sim.coders[i].thread, NULL, routine, &sim.coders[i]);
		i++;
	}
	i = 0;
	while (i < sim.number_of_coders)
		pthread_join(sim.coders[i++].thread, NULL);
	pthread_join(monitor_thread, NULL);
	// destroy(simulation, coders, dongles, monitor_thread);

	return 0;
}
