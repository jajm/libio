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

#ifndef libio_value_h_included
#define libio_value_h_included

#include <lua.h>

typedef enum {
	IO_VALUE_NIL,
	IO_VALUE_BOOLEAN,
	IO_VALUE_INTEGER,
	IO_VALUE_NUMBER,
	IO_VALUE_STRING,
	IO_VALUE_CFUNCTION,
	IO_VALUE_TABLE
} io_value_type_t;

typedef struct io_value_s io_value_t;

io_value_t * io_value_nil(void);

io_value_t * io_value_boolean(int b);
int io_value_boolean_set(io_value_t *v, int b);
int io_value_boolean_get(const io_value_t *v);

io_value_t * io_value_integer(lua_Integer i);
int io_value_integer_set(io_value_t *v, lua_Integer i);
lua_Integer io_value_integer_get(const io_value_t *v);

io_value_t * io_value_number(lua_Number n);
int io_value_number_set(io_value_t *v, lua_Number n);
lua_Number io_value_number_get(const io_value_t *v);

io_value_t * io_value_string(const char *s);
int io_value_string_set(io_value_t *v, const char *s);
const char * io_value_string_get(const io_value_t *v);

io_value_t * io_value_cfunction(lua_CFunction f);
int io_value_cfunction_set(io_value_t *v, lua_CFunction f);
lua_CFunction io_value_cfunction_get(const io_value_t *v);

io_value_t * io_value_table(void);
int io_value_table_set(io_value_t *v, io_value_t *k, io_value_t *val);
io_value_t * io_value_table_get(const io_value_t *v, io_value_t *k);

int io_value_isa(const io_value_t *v, io_value_type_t t);
io_value_type_t io_value_type(const io_value_t *v);

io_value_t * io_value_copy(io_value_t *v);

void io_value_free(io_value_t *v);

#endif /* ! libio_value_h_included */

