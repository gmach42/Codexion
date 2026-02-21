#include "codexion.h"

// AUTHORIZED :
// pthread_create, pthread_join, pthread_mutex_init,
// pthread_mutex_lock,
// pthread_mutex_unlock, pthread_mutex_destroy,
// pthread_cond_init,
// pthread_cond_wait, pthread_cond_timedwait,
// pthread_cond_broadcast,
// pthread_cond_destroy, gettimeofday, usleep, write,
// malloc, free,
// printf, fprintf, strcmp, strlen, atoi, memset


// ARGS :
// number_of_coders time_to_burnout time_to_compile time_to_debug
// time_to_refactor number_of_compiles_required dongle_cooldown scheduler

void* print_id(void* arg) {
	int id = *(int *)arg;
    printf("Hello from thread %d!\n", id);
    return NULL;
}

void* create_coder_thread(pthread_t *threads, int nb_coder)
{
	int i;

	i = 1;
	int ids[nb_coder];

	while (i <= nb_coder)
	{
		ids[i] = i;
		pthread_create(&threads[i], NULL, print_id, &ids[i]);
		i++;
	}
	return threads;
}

int main(int argc, char **argv)
{

	(void)argc;
	(void)argv;

	t_simulation	simulation;

	if (argc != 9)
	{
		printf("Expecting 8 args received %d args", argc - 1);
		return (0);
	}

	

    return 0;
}
