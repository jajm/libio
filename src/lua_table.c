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
#include <stdio.h>
#include <string.h>
#include <libobject/object.h>
#include <libobject/boolean.h>
#include <libobject/integer.h>
#include <libobject/real.h>
#include <libobject/string.h>
#include <libgends/hash_map.h>
#include "nil.h"
#include "cfunction.h"
#include "lua_table.h"

static const char IO_LUA_TABLE_TYPE[] = "Io:lua_table";
static const unsigned int IO_LUA_TABLE_HASH_SIZE = 128;

unsigned int io_lua_table_string_hash(const char *s)
{
	unsigned int hash = 0;

	if (s != NULL) {
		int i = 1;
		while (*s != '\0') {
			hash += (*s) * i;
			i++;
			s++;
		}
	}

	return hash;
}

unsigned int io_lua_table_hash_callback(const object_t *o, unsigned int size)
{
	unsigned int hash = 0;

	if (object_isset(o)) {
		if (object_is_nil(o)) {
			hash = 0;
		} else if (object_is_boolean(o)) {
			hash = (unsigned int) (boolean_get(o) ? 1 : 0);
		} else if (object_is_integer(o)) {
			hash = (unsigned int) integer_get(o);
		} else if (object_is_real(o)) {
			hash = (unsigned int) real_get(o);
		} else if (object_is_string(o)) {
			const char *s = string_to_c_str(o);
			hash = io_lua_table_string_hash(s);
		} else if (object_is_cfunction(o)) {
			hash = (intptr_t) io_cfunction_get(o);
		} else if (object_is_lua_table(o)) {
			hash = (intptr_t) object_value(o);
		} else {
			fprintf(stderr, "Unknown type\n");
		}
	}

	return hash % size;
}

int io_lua_table_cmpkey_callback(const object_t *o1, const object_t *o2)
{
	int cmp = 0;

	if (o1 == NULL && o2 == NULL) {
		cmp = 0;
	} else if (o1 == NULL && o2 != NULL) {
		cmp = 1;
	} else if (o1 != NULL && o2 == NULL) {
		cmp = -1;
	} else if (object_type(o1) != object_type(o2)) {
		cmp = strcmp(object_type(o1), object_type(o2));
	} else {
		/* o1 and o2 are of the same type */
		if (object_is_nil(o1)) {
			cmp = 0;
		} else if (object_is_boolean(o1)) {
			int b1 = boolean_get(o1) ? 1 : 0;
			int b2 = boolean_get(o2) ? 1 : 0;
			cmp = b1 - b2;
		} else if (object_is_integer(o1)) {
			cmp = integer_get(o1) - integer_get(o2);
		} else if (object_is_real(o1)) {
			cmp = (int) (real_get(o1) - real_get(o2));
		} else if (object_is_string(o1)) {
			const char *s1 = string_to_c_str(o1);
			const char *s2 = string_to_c_str(o2);
			cmp = strcmp(s1, s2);
		} else if (object_is_cfunction(o1)) {
			cmp = (int) (io_cfunction_get(o1) - io_cfunction_get(o2));
		} else if (object_is_lua_table(o1)) {
			cmp = (int) (object_value(o1) - object_value(o2));
		} else {
			fprintf(stderr, "Unknown type\n");
		}
	}

	return cmp;
}

io_lua_table_t * io_lua_table(void)
{
	io_lua_table_t *lua_table;
	gds_hash_map_t *hash_map;

	hash_map = gds_hash_map_new(IO_LUA_TABLE_HASH_SIZE,
		(gds_hash_cb) io_lua_table_hash_callback,
		(gds_cmpkey_cb) io_lua_table_cmpkey_callback);
	lua_table = object_new(IO_LUA_TABLE_TYPE, hash_map);

	return lua_table;
}

int io_lua_table_set(io_lua_table_t *lua_table, object_t *key, object_t *val)
{
	gds_hash_map_t *hash_map;

	if (object_is_lua_table(lua_table)) {
		hash_map = object_value(lua_table);
		gds_hash_map_set(hash_map, key, val, (gds_free_cb)object_free);
	}

	return 0;
}

object_t * io_lua_table_get(io_lua_table_t *lua_table, object_t *key)
{
	gds_hash_map_t *hash_map;
	object_t *val = NULL;

	if (object_is_lua_table(lua_table)) {
		hash_map = object_value(lua_table);
		val = gds_hash_map_get(hash_map, key);
	}

	return val;
}

int object_is_lua_table(const object_t *o)
{
	if (object_isa(o, IO_LUA_TABLE_TYPE))
		return 1;

	return 0;
}
