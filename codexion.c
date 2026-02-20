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

	i = 0;
	int ids[nb_coder];

	while (i < nb_coder)
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

	int	number_of_coders = 5;
	int	time_to_burnout; // in ms if a coder did not start compiling within time_to_burnout -> burn out
	int	time_to_compile; // 
	int	time_to_debug;
	int	time_to_refactor;
	int	number_of_compiles_required;
	int	dongle_cooldown;
	char	*scheduler; // fifo (First In, First Out) ou edf (Earliest Deadline First) deadline = last_compile_start + time_to_burnout

	pthread_t threads[number_of_coders];

	// create number_of_coders threads
	create_coder_thread(threads, number_of_coders);

	// Wait for each thread to finish
    for (int i = 0; i < number_of_coders; i++) {
        pthread_join(threads[i], NULL);
	}
    return 0;
}
