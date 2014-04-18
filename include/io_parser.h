/*
 * Copyright 2013-2014 Julian Maurice
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

#ifndef libio_parser_h_included
#define libio_parser_h_included

#include <sds.h>
#include "io_config.h"

sds
io_parser_parse(
	const char *template,
	io_config_t *config
);

sds
io_parser_parse_filep(
	FILE *filep,
	io_config_t *config
);

sds
io_parser_parse_file(
	const char *filename,
	io_config_t *config
);

#endif /* ! libio_parser_h_included */

