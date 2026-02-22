#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h> // usleep, write
# include <stdio.h> // printf, fprintf
# include <pthread.h> // pthread functions
# include <sys/time.h> // gettimeofday
# include <stdlib.h> // malloc, free, atoi
# include <string.h> // memset
# include <limits.h> // INT_MAX, INT_MIN
# include <stdbool.h> // bool, true, false

typedef struct s_simulation
{
	t_coder			*coders;
	t_dongle		*dongles;
	int				number_of_coders;
	int				time_to_burnout;
	int				time_to_compile;
	int				time_to_debug;
	int				time_to_refactor;
	int				number_of_compiles;
	int				number_of_compiles_required;
	int				dongle_cooldown;
	char			*scheduler;
	struct timeval	start_time;
}	t_simulation;

/**
 *	@file 	coder.c
 *	@brief	Represent a coder trying to compile before burnout
 *
 *	@param	id		Coder's ID
 *	@param	thread	Thread associated to the coder
 *	@param	left	Pointer to the left dongle
 *	@param	right	Pointer to the right dongle
*/
typedef struct s_coder
{
	int			id;
	pthread_t	thread;

	t_dongle	*left;	/**< Pointer to the left dongle */
	t_dongle	*right;	/**< Pointer to the right dongle */
}	t_coder;

/**
 *	@file 	dongle.c
 *	@brief	Represent a shared USB dongle
 *
 *	One dongle at the left and right of each coders whose are sitted in cirlcle
 *	If only one coder is present, only one dongle will be available
 *	Dongle's priority is managed by a heapq schedule (FIFO or EDF)
 *
 * @param	mutex		Lock a dongle when it's used
 * @param	cond		Condition to lock the dongle (cooldown)
 * @param	available	True if the dongle is available, false if it's being used or in cooldown
 * @param	cooldown	Timestamp in ms of the cooldown
 * @param	schedule	Heapq storing coders for each dongle
*/
typedef struct s_dongle
{
	pthread_mutex_t	mutex;	/**< Protects access to dongle state */
	pthread_cond_t	cond;	/**< Signal waiting coders */

	bool			available;	/**< Boolean indicating if the dongle is available */
	long			cooldown;	/**< Timestamp (ms) when dongle is reusable */

	t_heapq			*schedule; /**< Heapq of waiting coders */
}	t_dongle;

/**
 *	@file 	scheduler.c
 *	@brief	Represent a schedule of a dongle
 *
 * @param coder_id		Coder's ID
 * @param arrival_time	Integer
 * @param deadline		Integer representing the time before burnout
*/
typedef struct s_heapq
{
	int		coder_id; /**< Coder ID */
	int		arrival_time; /**< Arrival time in ms */
	int		deadline; /**< deadline = last_compile_start + time_to_burnout */
}	t_heapq;

/* Init functions */
t_coder			coder_init(t_coder	*coders);
t_coder			*coders_init(t_simulation simulation);
t_dongle		dongle_init(t_simulation simulation);
t_dongle		*dongles_init(t_simulation simulation);
t_simulation	simulation_init(char **argv);
pthread_t		monitor_init(t_simulation simulation);

/* Dongle functions */
int			can_take_dongle(t_coder *coder, t_dongle *dongle);
void		take_dongle(t_coder *coder, t_dongle *dongle);
void		release_dongle(t_coder *coder, t_dongle *dongle);
void		try_to_take_dongles(t_simulation simulation, t_coder *coder);
void		update_dongle_cooldown(t_dongle *dongle);

/* Coder functions */
void		routine(t_simulation simulation, t_coder *coder);
bool		is_burned(t_simulation simulation);
bool 		has_compiled_enough(t_simulation simulation);
void		compile(t_simulation simulation, t_coder *coder);
void		debug(t_simulation simulation, t_coder *coder);
void		refactor(t_simulation simulation, t_coder *coder);

/* Monitor functions */
void		*monitor_routine(void *arg);

/* Destroy functions */
void		destroy(t_simulation simulation, t_coder *coders, t_dongle *dongles, pthread_t monitor_thread);
void		destroy_coders(t_coder *coders, int number_of_coders);
void		destroy_dongles(t_dongle *dongles, int number_of_coders);
void		destroy_monitor(pthread_t monitor_thread);

#endif
