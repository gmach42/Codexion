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

# define INIT 0;
# define COMPILE 1;
# define DEBUG 2;
# define REFACTOR 4;
# define BURNOUT 8;

// Alias for timeval and timespec
typedef struct timeval	t_timeval;
typedef struct timespec t_timespec;

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
	int			coder_id; /**< Coder ID */
	t_timeval	arrival_time; /**< Arrival time in ms */
	t_timespec	deadline; /**< deadline = last_compile_start + time_to_burnout */
}	t_heapq;

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

	t_heapq			*schedule; /**< Heapq of waiting coders */
}	t_dongle;

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
	int			state;
	pthread_t	thread;

	t_dongle	*left;	/**< Pointer to the left dongle */
	t_dongle	*right;	/**< Pointer to the right dongle */
}	t_coder;

typedef struct s_simulation
{
	t_coder		*coders;
	t_dongle	*dongles;

	int			number_of_coders;
	int			number_of_compiles;
	int			number_of_compiles_required;

	int			time_to_burnout;
	int			time_to_debug;
	int			time_to_compile;
	int			time_to_refactor;
	int			dongle_cooldown;
	int			start_time;

}	t_simulation;

void	compile(t_simulation *sim, t_coder *coder);
void	debug(t_simulation *sim, t_coder *coder);
void	refactor(t_simulation *sim, t_coder *coder);
void	routine(t_simulation *sim, t_coder *coder);

void	dongle_take(t_simulation *sim, t_dongle *dongle, t_coder *coder);
void	dongle_release(t_simulation *sim, t_dongle *dongle);

t_simulation	simulation_init(char **argv);
void	coders_init(t_simulation *sim);
void	dongles_init(t_simulation *sim);

bool	simulation_stopped(t_simulation *sim);
long	time_since_start(t_simulation *sim);

#endif
