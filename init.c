#include "codexion.h"

t_simulation	simulation_init(char **argv)
{
	t_simulation	simulation;

	simulation.number_of_coders = atoi(argv[1]);
	simulation.time_to_burnout = atoi(argv[2]);
	simulation.time_to_compile = atoi(argv[3]);
	simulation.time_to_debug = atoi(argv[4]);
	simulation.time_to_refactor = atoi(argv[5]);
	simulation.number_of_compiles_required = atoi(argv[6]);
	simulation.dongle_cooldown = atoi(argv[7]);
	simulation.scheduler = argv[8];

	return (simulation);
}

t_coder	*coders_init(t_simulation simulation)
{
	t_coder		*coders;
	t_dongle	*dongles;
	int	i;

	coders = malloc(sizeof(t_coder) * simulation.number_of_coders);
	if (!coders)
		return (NULL);
	while (i < simulation.number_of_coders)
	{
		coders[i] = coder_init(simulation, &simulation.dongles[i], &simulation.dongles[(i + 1) % simulation.number_of_coders]);
		i++;
	}
	return (coders);
}

t_coder	coder_init(t_simulation simulation, t_dongle *dongle_left, t_dongle *dongle_right)
{
	t_coder	coder;

	coder.id = simulation.number_of_coders;
	coder.left = dongle_left;
	coder.right = dongle_right;
	return (coder);
}

t_dongle	*dongles_init(t_simulation simulation)
{
	t_dongle	*dongles;
	int			i;

	dongles = malloc(sizeof(t_dongle) * simulation.number_of_coders);
	if (!dongles)
		return (NULL);
	while (i < simulation.number_of_coders)
	{
		dongles[i] = dongle_init(simulation);
		i++;
	}
	return (dongles);
}

t_dongle	dongle_init(t_simulation simulation)
{
	t_dongle		dongle;
	pthread_cond_t	cond;
	pthread_mutex_t	mutex;
	t_heapq			*waiting_q;

	waiting_q = malloc(sizeof(t_heapq) * 2);
	if (!waiting_q)
		return (NULL);
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
	dongle.available = 1;
	dongle.cond = cond;
	dongle.cooldown = 0;
	dongle.mutex = mutex;
	dongle.schedule = waiting_q;

	return &dongle;
}
