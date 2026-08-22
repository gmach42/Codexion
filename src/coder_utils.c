/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gmach <gmach@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/22 00:00:00 by gmach             #+#    #+#             */
/*   Updated: 2026/08/22 00:00:00 by gmach            ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	enqueue_dongles(t_coder *coder, size_t arrival)
{
	dongle_enqueue(coder->sim, coder->left, coder, arrival);
	dongle_enqueue(coder->sim, coder->right, coder, arrival);
}
