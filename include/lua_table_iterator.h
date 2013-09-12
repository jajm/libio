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

#ifndef libio_lua_table_iterator_h_included
#define libio_lua_table_iterator_h_included

#include <libobject/object.h>
#include "lua_table.h"

typedef struct io_lua_table_iterator_s io_lua_table_iterator_t;

io_lua_table_iterator_t * io_lua_table_iterator(io_lua_table_t *lua_table);

int io_lua_table_iterator_reset(io_lua_table_iterator_t *it);
int io_lua_table_iterator_step(io_lua_table_iterator_t *it);
object_t * io_lua_table_iterator_getkey(io_lua_table_iterator_t *it);
object_t * io_lua_table_iterator_getvalue(io_lua_table_iterator_t *it);

void io_lua_table_iterator_free(io_lua_table_iterator_t *it);

#endif /* ! libio_lua_table_iterator_h_included */

