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

#ifndef libio_lua_table_h_included
#define libio_lua_table_h_included

#include <libobject/object.h>
#include <libobject/hash.h>
#include <libobject/array.h>

typedef object_t io_lua_table_t;

io_lua_table_t * io_lua_table(void);

int io_lua_table_set(io_lua_table_t *lua_table, object_t *key, object_t *val);
object_t * io_lua_table_get(io_lua_table_t *lua_table, object_t *key);

int object_is_lua_table(const object_t *o);

#endif /* ! libio_lua_table_h_included */

