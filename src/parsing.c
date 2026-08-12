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

/**
 * @brief Return true iff `str` is a non-empty sequence of decimal digits.
 *        Rejects negative numbers, floats, hex, and any other malformed input.
 */
static bool	is_valid_uint(const char *str)
{
	int	i;

	if (!str || str[0] == '\0')
		return (false);
	i = 0;
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (false);
		i++;
	}
	return (true);
}

int	parse_int(char *str)
{
	if (!is_valid_uint(str))
	{
		fprintf(stderr, "Invalid argument: %s\n", str);
		exit(EXIT_FAILURE);
	}
	return (atoi(str));
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
