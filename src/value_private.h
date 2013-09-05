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

#ifndef libio_value_private_h_included
#define libio_value_private_h_included

#include <lua.h>
#include <libgends/hash_map.h>

struct io_value_s {
	io_value_type_t type;
	union {
		int b;
		lua_Integer i;
		lua_Number n;
		char *s;
		lua_CFunction f;
		gds_hash_map_t *t;
	} data;
};

#endif /* ! libio_value_private_h_included */

