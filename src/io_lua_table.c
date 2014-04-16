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
#include <stdbool.h>
#include <libgends/hash_map.h>
#include <libgends/hash_functions.h>
#include <embody/embody.h>
#include <sds.h>
#include "io_lua_value.h"
#include "io_lua_table.h"
#include "io_embody.h"

static const unsigned long IO_LUA_TABLE_HASH_SIZE = 128;

static unsigned long io_lua_table_hash_callback(void **data, unsigned long size)
{
	unsigned long hash = 0;
	io_lua_value_t lua_value;

	lua_value.type = LUA_VALUE_TYPE_NONE;
	io_emb_data_to_lua_value(data, &lua_value);

	if (lua_value.type != LUA_VALUE_TYPE_NONE) {
		switch (lua_value.type) {
			case LUA_VALUE_TYPE_BOOLEAN:
				hash = lua_value.value.boolean ? 1 : 0;
				break;
			case LUA_VALUE_TYPE_INTEGER:
				hash = lua_value.value.integer;
				break;
			case LUA_VALUE_TYPE_UNSIGNED:
				hash = lua_value.value.unsignd;
				break;
			case LUA_VALUE_TYPE_NUMBER:
				hash = lua_value.value.number;
				break;
			case LUA_VALUE_TYPE_STRING:
				hash = gds_hash_djb2(lua_value.value.string);
				break;
			case LUA_VALUE_TYPE_CFUNCTION:
				hash = (unsigned long) lua_value.value.cfunction;
				break;
			case LUA_VALUE_TYPE_LIST:
				hash = (unsigned long) lua_value.value.list;
				break;
			case LUA_VALUE_TYPE_TABLE:
				hash = (unsigned long) lua_value.value.table;
				break;
			case LUA_VALUE_TYPE_LIGHTUSERDATA:
				hash = (unsigned long) lua_value.value.lightuserdata;
				break;
			default:
				hash = 0;
		}
	}

	return hash % size;
}

static int io_lua_table_cmpkey_callback(void **data1, void **data2)
{
	int cmp = 0;

	if (data1 == NULL && data2 == NULL) {
		cmp = 0;
	} else if (data1 == NULL && data2 != NULL) {
		cmp = 1;
	} else if (data1 != NULL && data2 == NULL) {
		cmp = -1;
	} else if (emb_type(data1) != emb_type(data2)) {
		cmp = strcmp(emb_type_name(data1), emb_type_name(data2));
	} else {
		io_lua_value_t v1, v2;

		v1.type = v2.type = LUA_VALUE_TYPE_NONE;
		io_emb_data_to_lua_value(data1, &v1);
		io_emb_data_to_lua_value(data2, &v2);
		if (v1.type != LUA_VALUE_TYPE_NONE) {
			switch (v1.type) {
				case LUA_VALUE_TYPE_BOOLEAN:
					cmp = v1.value.boolean - v2.value.boolean;
					break;
				case LUA_VALUE_TYPE_INTEGER:
					cmp = v1.value.integer - v2.value.integer;
					break;
				case LUA_VALUE_TYPE_UNSIGNED:
					cmp = v1.value.unsignd - v2.value.unsignd;
					break;
				case LUA_VALUE_TYPE_NUMBER:
					cmp = v1.value.number - v2.value.number;
					break;
				case LUA_VALUE_TYPE_STRING:
					cmp = strcmp(v1.value.string, v2.value.string);
					break;
				case LUA_VALUE_TYPE_CFUNCTION:
					cmp = v1.value.cfunction - v2.value.cfunction;
					break;
				case LUA_VALUE_TYPE_LIST:
					cmp = v1.value.list - v2.value.list;
					break;
				case LUA_VALUE_TYPE_TABLE:
					cmp = v1.value.table - v2.value.table;
					break;
				case LUA_VALUE_TYPE_LIGHTUSERDATA:
					cmp = v1.value.lightuserdata - v2.value.lightuserdata;
					break;
				default:
					cmp = 0;
			}
		} else {
			cmp = data1 - data2;
		}
	}

	return cmp;
}

io_lua_table_t * io_lua_table_new(void)
{
	io_lua_table_t *lua_table;

	lua_table = gds_hash_map_new(IO_LUA_TABLE_HASH_SIZE,
		io_lua_table_hash_callback, io_lua_table_cmpkey_callback,
		NULL, emb_container_free, emb_container_free);

	return lua_table;
}
