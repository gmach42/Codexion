# include "codexion.h"

void	compile(t_simulation *sim, t_coder *coder)
{
	printf("%ld %d is compiling\n", time_since_start(sim).tv_sec, coder->id);
	coder->state = COMPILE;
	usleep(sim->time_to_compile * 1000);
}

void	debug(t_simulation *sim, t_coder *coder)
{
	printf("%ld %d is debugging\n", time_since_start(sim).tv_sec, coder->id);
	coder->state = DEBUG;
	usleep(sim->time_to_compile * 1000);
}

void	refactor(t_simulation *sim, t_coder *coder)
{
	printf("%ld %d is refactoring\n", time_since_start(sim).tv_sec, coder->id);
	coder->state = REFACTOR;
	usleep(sim->time_to_compile * 1000);
}

void	routine(t_simulation *sim, t_coder *coder)
{
	printf("start routine\n");
	dongle_take(sim, coder->left, coder);
	dongle_take(sim, coder->right, coder);
	compile(sim, coder);
	debug(sim, coder);
	refactor(sim, coder);
}

