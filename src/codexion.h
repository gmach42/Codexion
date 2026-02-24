/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gildas <gildas@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 18:40:41 by gildas            #+#    #+#             */
/*   Updated: 2026/02/24 18:45:02 by gildas           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>	  // usleep, write
# include <stdio.h>	  // printf, fprintf
# include <pthread.h>  // pthread functions
# include <sys/time.h> // gettimeofday
# include <stdlib.h>	  // malloc, free, atoi
# include <string.h>	  // memset
# include <limits.h>	  // INT_MAX, INT_MIN
# include <stdbool.h>  // bool, true, false

# define INIT 0;
# define COMPILE 1;
# define DEBUG 2;
# define REFACTOR 4;
# define BURNOUT 8;

// Alias for timeval and timespec
typedef struct timeval	t_timeval;
typedef struct timespec	t_timespec;
typedef pthread_mutex_t	mutex_t;
typedef pthread_cond_t	cond_t;

typedef struct s_sim	t_sim;
typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;

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
	int			coder_id;			/**< Coder ID */
	t_timeval	arrival_time; /**< Arrival time in ms */
	t_timespec	deadline;	/**< deadline = last_compile_start + time_to_burnout */
} t_heapq;

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
	mutex_t			mutex; /**< Protects access to dongle state */
	pthread_cond_t	cond;   /**< Signal waiting coders */

	bool			available; /**< Boolean indicating if the dongle is available */

	t_heapq			*schedule; /**< Heapq of waiting coders */
} t_dongle;

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

	size_t		last_compile_time;
	mutex_t		time_mutex;

	pthread_t	thread;
	t_sim		*sim;

	t_dongle	*left;	 /**< Pointer to the left dongle */
	t_dongle	*right; /**< Pointer to the right dongle */
} t_coder;

typedef struct s_sim
{
	t_coder 	*coders;
	t_dongle 	*dongles;

	pthread_t	monitor;
	mutex_t		stop_mutex;
	mutex_t		print_mutex;
	bool		stop;

	int 		number_of_coders;
	int 		number_of_compiles;
	int 		number_of_compiles_required;

	size_t		time_to_burnout;
	size_t		time_to_debug;
	size_t		time_to_compile;
	size_t		time_to_refactor;
	size_t		dongle_cooldown;
	size_t		start_time;

} t_sim;

void	compile(t_sim *sim, t_coder *coder);
void	debug(t_sim *sim, t_coder *coder);
void	refactor(t_sim *sim, t_coder *coder);
void	*routine(void *arg);
void	safe_print(t_sim *sim, int id, char *msg);
bool	sim_stopped(t_sim *sim);

void	dongle_take(t_sim *sim, t_dongle *dongle, t_coder *coder);
void	dongle_release(t_sim *sim, t_dongle *dongle);

void	simulation_init(t_sim *sim, char **argv);
void	coders_init(t_sim *sim);
void	dongles_init(t_sim *sim);

size_t	time_since_start(t_sim *sim);
size_t	get_current_time(void);

#endif
