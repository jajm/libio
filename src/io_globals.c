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

static io_config_t * io_default_config = NULL;

io_config_t * io_globals_get_default_config(void)
{
	if (!io_default_config) {
		io_default_config = io_config_new_default();
	}

	return io_default_config;
}

void io_globals_free(void)
{
	io_config_free(io_default_config);
	io_default_config = NULL;
}
