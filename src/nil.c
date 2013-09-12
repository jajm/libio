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

#include <stdlib.h>
#include <libobject/object.h>
#include "nil.h"

static const char IO_NIL_TYPE[] = "Io:nil";

io_nil_t * io_nil(void)
{
	io_nil_t *nil;

	nil = object_new(IO_NIL_TYPE, NULL);

	return nil;
}

int object_is_nil(const object_t *o)
{
	if (object_isa(o, IO_NIL_TYPE))
		return 1;

	return 0;
}
