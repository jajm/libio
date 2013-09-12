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

#ifndef libio_cfunction_h_included
#define libio_cfunction_h_included

#include <lua.h>
#include <libobject/object.h>

typedef object_t io_cfunction_t;

io_cfunction_t * io_cfunction(lua_CFunction f);
int io_cfunction_set(io_cfunction_t *cf, lua_CFunction f);
lua_CFunction io_cfunction_get(const io_cfunction_t *cf);

int object_is_cfunction(const object_t *object);

#endif /* ! libio_cfunction_h_included */

