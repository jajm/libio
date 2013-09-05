/*
 * Copyright 2013 Julian Maurice
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

#ifndef libio_template_h_included
#define libio_template_h_included

#include "value.h"

typedef struct io_template_s io_template_t;

io_template_t *
io_template_new(
	const char *template
);

io_template_t *
io_template_new_from_file(
	const char *filename
);

void
io_template_param(
	io_template_t *T,
	const char *name,
	io_value_t *value
);

char *
io_template_render(
	io_template_t *T
);

void
io_template_free(
	io_template_t *T
);

#endif /* ! libio_template_h_included */

