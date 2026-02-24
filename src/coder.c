#include "codexion.h"

void	compile(t_sim *sim, t_coder *coder)
{
	safe_print(sim, coder->id, "is compiling");
	pthread_mutex_lock(&coder->time_mutex);
	coder->last_compile_time = get_current_time();
	pthread_mutex_unlock(&coder->time_mutex);
	usleep(sim->time_to_compile * 1000);
	dongle_release(sim, coder->left);
	dongle_release(sim, coder->right);
}

void	debug(t_sim *sim, t_coder *coder)
{
	safe_print(sim, coder->id, "is debugging");
	usleep(sim->time_to_debug * 1000);
}

void	refactor(t_sim *sim, t_coder *coder)
{
	safe_print(sim, coder->id, "is refactoring");
	usleep(sim->time_to_refactor * 1000);
}

void	*routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->id % 2 != 0)
		usleep(1000);
	while (!sim_stopped(coder->sim))
	{
		dongle_take(coder->sim, coder->left, coder);
		dongle_take(coder->sim, coder->right, coder);
		if (sim_stopped(coder->sim))
			break ;
		compile(coder->sim, coder);
		if (sim_stopped(coder->sim))
			break ;
		debug(coder->sim, coder);
		if (sim_stopped(coder->sim))
			break ;
		refactor(coder->sim, coder);
		coder->sim->number_of_compiles++;
	}
	return (NULL);
}
