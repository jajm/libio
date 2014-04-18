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

#include "io_globals.h"
#include "io_embody.h"

static int io_initialized = 0;
void io_initialize(void)
{
	if (!io_initialized) {
		io_emb_initialize();

		io_initialized = 1;
	}
}

void io_finalize(void)
{
	io_globals_free();
	io_initialized = 0;
}
