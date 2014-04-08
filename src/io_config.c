/*
 * Copyright 2014 Julian Maurice
 *
 * This file is part of libio
 *
 * libio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libio.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include "io_config.h"

io_config_t * io_config_new(void)
{
	io_config_t *config;

	config = malloc(sizeof(io_config_t));

	/* Set defaults */
	config->start_tag = sdsnew("#{");
	config->end_tag = sdsnew("}#");
	config->directories = gds_slist_new(sdsfree);
	gds_slist_push(config->directories, sdsnew("."));

	return config;
}

void io_config_free(io_config_t *config)
{
	if (config) {
		sdsfree(config->start_tag);
		sdsfree(config->end_tag);
		gds_slist_free(config->directories);
		free(config);
	}
}
