# include "codexion.h"

// Function to swap two integers
void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

// add a new coder to the heapq
void    heappush(t_heapq *schedule, t_coder *coder)
{
	// TODO add a new coder to the heapq
}

// remove a coder from the heapq
t_coder   *heappop(t_heapq *schedule, t_coder *coder, char *mod)
{
	// TODO remove the more the first coder from the heapq
}

t_heapq	heapify(t_coder *coders)
{
	// TODO transform a list of coder into a heapq?
}

// Find the top prio coder of the heapq
t_coder	*heappeak(t_heapq *schedule, t_dongle dongle)
{
	// TODO
}



