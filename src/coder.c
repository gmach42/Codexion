#include "codexion.h"

void compile(t_sim *sim, t_coder *coder)
{
	printf("%ld %d is compiling\n", time_since_start(sim), coder->id);
	coder->state = COMPILE;
	usleep(sim->time_to_compile);
	dongle_release(sim, coder->left);
	dongle_release(sim, coder->right);
}

void debug(t_sim *sim, t_coder *coder)
{
	printf("%ld %d is debugging\n", time_since_start(sim), coder->id);
	coder->state = DEBUG;
	usleep(sim->time_to_compile);
}

void refactor(t_sim *sim, t_coder *coder)
{
	printf("%ld %d is refactoring\n", time_since_start(sim), coder->id);
	coder->state = REFACTOR;
	usleep(sim->time_to_compile);
}

void *routine(void *arg)
{
	t_coder *coder;

	coder = (t_coder *)arg;
	printf("test\n");

	while (!coder->sim->stop)
	{
		printf("start routine\n");
		printf("sim %X, dongle %X, coder %X\n", coder->sim, coder->left, coder);
		dongle_take(coder->sim, coder->left, coder);
		dongle_take(coder->sim, coder->right, coder);
		compile(coder->sim, coder);
		debug(coder->sim, coder);
		refactor(coder->sim, coder);
		coder->sim->number_of_compiles++;
		printf("Compilation num %d complete!\n\n", coder->sim->number_of_compiles);
	}
	return (NULL);
}
