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

#include <stdlib.h>
#include <libgends/slist.h>
#include <libgends/dlist.h>
#include <libgends/hash_map.h>
#include <libgends/hash_map_fast.h>
#include <libgends/hash_map_keyin.h>
#include <libgends/hash_map_keyin_fast.h>
#include <sds.h>
#include <embody/embody.h>
#include <lua.h>
#include "lua_value.h"

static void io_bool_to_lua_value(_Bool *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_BOOLEAN;
	lua_value->value.boolean = *data;
}

static void io_short_to_lua_value(short *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_INTEGER;
	lua_value->value.integer = *data;
}

static void io_int_to_lua_value(int *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_INTEGER;
	lua_value->value.integer = *data;
}

static void io_long_to_lua_value(long *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_INTEGER;
	lua_value->value.integer = *data;
}

static void io_longlong_to_lua_value(long long *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_INTEGER;
	lua_value->value.integer = *data;
}

static void io_int8_to_lua_value(int8_t *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_INTEGER;
	lua_value->value.integer = *data;
}

static void io_int16_to_lua_value(int16_t *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_INTEGER;
	lua_value->value.integer = *data;
}

static void io_int32_to_lua_value(int32_t *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_INTEGER;
	lua_value->value.integer = *data;
}

static void io_int64_to_lua_value(int64_t *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_INTEGER;
	lua_value->value.integer = *data;
}

static void io_float_to_lua_value(float *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_NUMBER;
	lua_value->value.number = *data;
}

static void io_double_to_lua_value(double *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_NUMBER;
	lua_value->value.number = *data;
}

static void io_longdouble_to_lua_value(long double *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_NUMBER;
	lua_value->value.number = *data;
}

static void io_string_to_lua_value(char *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_STRING;
	lua_value->value.string = data;
}

static void io_cfunction_to_lua_value(void *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_CFUNCTION;
	lua_value->value.cfunction = data;
}

static void io_list_to_lua_value(void *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_LIST;
	lua_value->value.list = data;
}

static void io_table_to_lua_value(void *data, io_lua_value_t *lua_value)
{
	lua_value->type = LUA_VALUE_TYPE_TABLE;
	lua_value->value.table = data;
}

static void io_emb_register_callback(emb_type_t *type, const char *name,
	void *callback)
{
	void *cb;

	cb = emb_type_get_callback(type, name);
	if (!cb) {
		emb_type_register_callback(type, name, callback);
	}
}

static void io_emb_register_to_lua_value(emb_type_t *type, void *callback)
{
	io_emb_register_callback(type, "io_to_lua_value", callback);
}

static void io_emb_register_gds_iterator(emb_type_t *type, void *callback)
{
	io_emb_register_callback(type, "gds_iterator", callback);
}

static void io_emb_initialize_default_types(void)
{
	emb_type_t *type;

	type = emb_type_get("bool");
	io_emb_register_to_lua_value(type, io_bool_to_lua_value);

	type = emb_type_get("short");
	io_emb_register_to_lua_value(type, io_short_to_lua_value);

	type = emb_type_get("int");
	io_emb_register_to_lua_value(type, io_int_to_lua_value);

	type = emb_type_get("long");
	io_emb_register_to_lua_value(type, io_long_to_lua_value);

	type = emb_type_get("longlong");
	io_emb_register_to_lua_value(type, io_longlong_to_lua_value);

	type = emb_type_get("int8");
	io_emb_register_to_lua_value(type, io_int8_to_lua_value);

	type = emb_type_get("int16");
	io_emb_register_to_lua_value(type, io_int16_to_lua_value);

	type = emb_type_get("int32");
	io_emb_register_to_lua_value(type, io_int32_to_lua_value);

	type = emb_type_get("int64");
	io_emb_register_to_lua_value(type, io_int64_to_lua_value);

	type = emb_type_get("float");
	io_emb_register_to_lua_value(type, io_float_to_lua_value);

	type = emb_type_get("double");
	io_emb_register_to_lua_value(type, io_double_to_lua_value);

	type = emb_type_get("longdouble");
	io_emb_register_to_lua_value(type, io_longdouble_to_lua_value);
}

static void io_emb_initialize_string_types(void)
{
	emb_type_t *type;

	type = emb_type_get("sds");
	io_emb_register_to_lua_value(type, io_string_to_lua_value);
	io_emb_register_callback(type, "free", sdsfree);

	type = emb_type_get("string");
	io_emb_register_to_lua_value(type, io_string_to_lua_value);
	io_emb_register_callback(type, "free", free);
}

static void io_emb_initialize_lua_types(void)
{
	emb_type_t *type;

	type = emb_type_get("cfunction");
	io_emb_register_to_lua_value(type, io_cfunction_to_lua_value);
}

static void io_emb_initialize_gds_types(void)
{
	emb_type_t *type;

	type = emb_type_get("gds_slist");
	io_emb_register_to_lua_value(type, io_list_to_lua_value);
	io_emb_register_gds_iterator(type, gds_slist_iterator_new);
	io_emb_register_callback(type, "free", gds_slist_free);

	type = emb_type_get("gds_dlist");
	io_emb_register_to_lua_value(type, io_list_to_lua_value);
	io_emb_register_gds_iterator(type, gds_dlist_iterator_new);
	io_emb_register_callback(type, "free", gds_dlist_free);

	type = emb_type_get("gds_hash_map");
	io_emb_register_to_lua_value(type, io_table_to_lua_value);
	io_emb_register_gds_iterator(type, gds_hash_map_iterator_new);
	io_emb_register_callback(type, "free", gds_hash_map_free);

	type = emb_type_get("gds_hash_map_fast");
	io_emb_register_to_lua_value(type, io_table_to_lua_value);
	io_emb_register_gds_iterator(type, gds_hash_map_fast_iterator_new);
	io_emb_register_callback(type, "free", gds_hash_map_fast_free);

	type = emb_type_get("gds_hash_map_keyin");
	io_emb_register_to_lua_value(type, io_table_to_lua_value);
	io_emb_register_gds_iterator(type, gds_hash_map_keyin_iterator_new);
	io_emb_register_callback(type, "free", gds_hash_map_keyin_free);

	type = emb_type_get("gds_hash_map_keyin_fast");
	io_emb_register_to_lua_value(type, io_table_to_lua_value);
	io_emb_register_gds_iterator(type,
		gds_hash_map_keyin_fast_iterator_new);
	io_emb_register_callback(type, "free", gds_hash_map_keyin_fast_free);
}

void io_emb_initialize(void)
{
	io_emb_initialize_default_types();
	io_emb_initialize_string_types();
	io_emb_initialize_lua_types();
	io_emb_initialize_gds_types();
}

void io_emb_data_to_lua_value(void **data, io_lua_value_t *lua_value)
{
	emb_type_t *type;
	void (*to_lua_value_cb)(void *, io_lua_value_t *);

	if (data && lua_value) {
		type = emb_type(data);
		to_lua_value_cb = emb_type_get_callback(type, "io_to_lua_value");
		if (to_lua_value_cb) {
			to_lua_value_cb(*data, lua_value);
		}
	}
}
