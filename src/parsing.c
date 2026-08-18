/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/25 17:30:20 by gmach             #+#    #+#             */
/*   Updated: 2026/08/18 18:17:26 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

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
		return (FIFO);
	else if (strcmp(str, "edf") == 0)
		return (EDF);
	else
	{
		fprintf(stderr, "Invalid scheduler: %s\n", str);
		exit(EXIT_FAILURE);
	}
}
