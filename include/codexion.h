/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:30:08 by gmach             #+#    #+#             */
/*   Updated: 2026/08/18 18:17:39 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <unistd.h>	  // usleep, write
# include <stdio.h>	  // printf, fprintf
# include <pthread.h>  // pthread functions
# include <sys/time.h> // gettimeofday
# include <stdlib.h>	  // malloc, free, atoi
# include <string.h>	  // strcmp
# include <stdbool.h>  // bool, true, false

// Alias for timespec
typedef struct timespec	t_timespec;
typedef pthread_mutex_t	t_mutex;
typedef pthread_cond_t	t_cond;

typedef struct s_sim	t_sim;
typedef struct s_coder	t_coder;
typedef struct s_dongle	t_dongle;

typedef enum e_schedule
{
	FIFO,
	EDF
}	t_schedule;

# define GREEN "\033[0;32m"
# define BLUE "\033[0;36m"
# define YELLOW "\033[0;33m"
# define RED "\033[1;31m"
# define CYAN "\033[0;35m"
# define FLASH "\033[1;93m"
# define NC "\033[0m"

/**
 *	@file	heapq.c
 *	@brief	Node stored in a dongle's priority queue
 *
 *	@param	priority	Lower value = higher priority.
 *						FIFO: arrival timestamp (ms).
 *						EDF : last_compile_start + time_to_burnout (ms).
 *	@param	tiebreak	Arrival timestamp (ms), secondary sort key.
 *	@param	coder_id	ID of the requesting coder.
 */
typedef struct s_heap_node
{
	size_t	priority;
	size_t	tiebreak;
	int		coder_id;
}	t_hnode;

/**
 *	@file	heapq.c
 *	@brief	Min-heap (priority queue)
 *
 *	@param	nodes		heap nodes.
 *	@param	size		Number of elements currently in the heap.
 *	@param	capacity	Maximum number of elements (= number_of_coders).
 */
typedef struct s_heap
{
	t_hnode	*nodes;
	int		size;
	int		capacity;
}	t_heap;

/**
 *	@file 	dongle.c
 *	@brief	Represent a shared USB dongle
 *
 *	One dongle at the left and right of each coders whose are sitted in circle
 *	If only one coder is present, only one dongle will be available
 *	Dongle's priority is managed by a heapq schedule (FIFO or EDF)
 *
 * @param	mutex		Lock a dongle when it's used
 * @param	cond		Condition to lock the dongle (cooldown)
 * @param	available	true if available else false
 * @param	queue		Priority queue of coders waiting for this dongle
 */
typedef struct s_dongle
{
	t_mutex	mutex;	/**< Protects access to dongle state */
	t_cond	cond;	/**< Signal waiting coders */
	bool	available; /**< Boolean indicating if the dongle is available */
	size_t	cooldown; /**< Timestamp when the dongle will be available again */
	t_heap	queue;	/**< Priority queue of waiting coders */
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

	size_t		last_compile_time;
	int			compile_count;
	t_mutex		time_mutex;

	pthread_t	thread;
	t_sim		*sim;

	t_dongle	*left; /**< Pointer to the left dongle */
	t_dongle	*right; /**< Pointer to the right dongle */
}	t_coder;

typedef struct s_sim
{
	t_coder		*coders;
	t_dongle	*dongles;

	t_mutex		stop_mutex;
	t_mutex		print_mutex;
	bool		stop;
	t_schedule	dongle_schedule; /**< FIFO or EDF */
	int			nb_coders;
	int			nb_compiles;

	size_t		time_to_burnout;
	size_t		time_to_debug;
	size_t		time_to_compile;
	size_t		time_to_refactor;
	size_t		dongle_cd;
	size_t		start_time;

}	t_sim;

bool	compile(t_sim *sim, t_coder *coder);
bool	debug(t_sim *sim, t_coder *coder);
bool	refactor(t_sim *sim, t_coder *coder);
void	*routine(void *arg);
void	safe_print(t_sim *sim, int id, char *msg);
void	sim_stop_setter(t_sim *sim);
bool	sim_stop_getter(t_sim *sim);

void	dongle_enqueue(t_sim *sim, t_dongle *dongle, t_coder *coder, size_t t);
bool	dongle_wait_and_take(t_sim *sim, t_dongle *dongle, t_coder *coder);
void	dongle_release(t_sim *sim, t_dongle *dongle);
void	free_dongles(t_sim *sim);

void	heap_push(t_heap *heap, t_hnode node);
t_hnode	heap_pop(t_heap *heap);
bool	heap_top_is(t_heap *heap, int coder_id);

void	simulation_init(t_sim *sim, char **argv);
bool	coders_init(t_sim *sim);
bool	dongles_init(t_sim *sim);
void	*monitor_routine(void *arg);
void	launch_coders(t_sim *sim);
void	join_coders(t_sim *sim, pthread_t monitor_thread);

size_t	get_current_time(void);
void	better_sleep(t_sim *sim, size_t duration_ms);

int		parse_int(char *str);
int		parse_str(char *str);

#endif
