/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:30:20 by gmach             #+#    #+#             */
/*   Updated: 2026/02/25 18:48:43 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/**
 * @brief Files to parse arguments
 *
 * Args to parse include:
 * - number_of_coders (size_t)
 * - time_to_burnout (size_t)
 * - time_to_compile (size_t)
 * - time_to_debug (size_t)
 * - time_to_refactor (size_t)
 * - number_of_compiles_required (int)
 * - dongle_cooldown (size_t)
 * - scheduler (char *) 'fifo' or 'edf'
 */

// TODO Check if no negative and types as well if scheduler is fifo or edf

int	parse_int(char *str)
{
	int	result;

	result = atoi(str);
	if (result < 0)
	{
		fprintf(stderr, "Invalid argument: %s\n", str);
		exit(EXIT_FAILURE);
	}
	return (result);
}

int	parse_str(char *str)
{
	if (strcmp(str, "fifo") == 0)
		return (0);
	else if (strcmp(str, "edf") == 0)
		return (1);
	else
	{
		fprintf(stderr, "Invalid scheduler: %s\n", str);
		exit(EXIT_FAILURE);
	}
}
