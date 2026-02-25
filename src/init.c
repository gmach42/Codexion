#include "codexion.h"

void	simulation_init(t_sim *sim, char **argv)
{
	sim->number_of_coders = atoi(argv[1]);
	sim->time_to_burnout = atoi(argv[2]);
	sim->time_to_compile = atoi(argv[3]);
	sim->time_to_debug = atoi(argv[4]);
	sim->time_to_refactor = atoi(argv[5]);
	sim->number_of_compiles_required = atoi(argv[6]);
	sim->dongle_cooldown = atoi(argv[7]);
	sim->start_time = get_current_time();
	sim->stop = false;
	sim->number_of_compiles = 0;
	pthread_mutex_init(&sim->stop_mutex, NULL);
	pthread_mutex_init(&sim->print_mutex, NULL);
	pthread_mutex_init(&sim->compile_mutex, NULL);
}

void	coder_init(t_sim *sim, t_coder *coder, int id, t_dongle *left, t_dongle *right)
{
	coder->id = id;
	coder->left = left;
	coder->right = right;
	coder->sim = sim;
	coder->last_compile_time = sim->start_time;
	pthread_mutex_init(&coder->time_mutex, NULL);
}

void	coders_init(t_sim *sim)
{
	t_coder		*coders;
	t_dongle	*dongles;
	int			i;

	i = 0;
	coders = malloc(sizeof(t_coder) * sim->number_of_coders);
	if (!coders)
		return ;
	while (i < sim->number_of_coders)
	{
		coder_init(sim, &coders[i], i, &sim->dongles[i], &sim->dongles[(i + 1) % sim->number_of_coders]);
		coders[i++].sim = sim;
	}
	sim->coders = coders;
}

void	dongle_init(t_dongle *dongle)
{
	t_heapq	*waiting_q;

	waiting_q = malloc(sizeof(t_heapq) * 2);
	if (!waiting_q)
	{
		printf("Error while creating dongles\n");
		exit(EXIT_FAILURE);
	}
	pthread_cond_init(&dongle->cond, NULL);
	pthread_mutex_init(&dongle->mutex, NULL);
	dongle->available = true;
	dongle->schedule = waiting_q;
}

void	dongles_init(t_sim *sim)
{
	t_dongle	*dongles;
	int			i;

	i = 0;
	dongles = malloc(sizeof(t_dongle) * sim->number_of_coders);
	if (!dongles)
	{
		printf("Error while creating dongles\n");
		exit(EXIT_FAILURE);
	}
	while (i < sim->number_of_coders)
	{
		dongle_init(&dongles[i]);
		i++;
	}
	sim->dongles = dongles;
}
