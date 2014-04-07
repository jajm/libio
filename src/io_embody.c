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

#define IO_TO_LUA_VALUE_FUNC(type_name, ctype, luatype, member) \
	static void io_##type_name##_to_lua_value(ctype *data, io_lua_value_t *value) \
	{ \
		value->type = luatype; \
		value->value.member = *data; \
	}

IO_TO_LUA_VALUE_FUNC(short, short, LUA_VALUE_TYPE_INTEGER, integer)
IO_TO_LUA_VALUE_FUNC(int, int, LUA_VALUE_TYPE_INTEGER, integer)
IO_TO_LUA_VALUE_FUNC(long, long, LUA_VALUE_TYPE_INTEGER, integer)
IO_TO_LUA_VALUE_FUNC(longlong, long long, LUA_VALUE_TYPE_INTEGER, integer)
IO_TO_LUA_VALUE_FUNC(ushort, unsigned short, LUA_VALUE_TYPE_UNSIGNED, unsignd)
IO_TO_LUA_VALUE_FUNC(uint, unsigned int, LUA_VALUE_TYPE_UNSIGNED, unsignd)
IO_TO_LUA_VALUE_FUNC(ulong, unsigned long, LUA_VALUE_TYPE_UNSIGNED, unsignd)
IO_TO_LUA_VALUE_FUNC(ulonglong, unsigned long long, LUA_VALUE_TYPE_UNSIGNED, unsignd)

IO_TO_LUA_VALUE_FUNC(int8, int8_t, LUA_VALUE_TYPE_INTEGER, integer)
IO_TO_LUA_VALUE_FUNC(int16, int16_t, LUA_VALUE_TYPE_INTEGER, integer)
IO_TO_LUA_VALUE_FUNC(int32, int32_t, LUA_VALUE_TYPE_INTEGER, integer)
IO_TO_LUA_VALUE_FUNC(int64, int64_t, LUA_VALUE_TYPE_INTEGER, integer)
IO_TO_LUA_VALUE_FUNC(uint8, uint8_t, LUA_VALUE_TYPE_UNSIGNED, unsignd)
IO_TO_LUA_VALUE_FUNC(uint16, uint16_t, LUA_VALUE_TYPE_UNSIGNED, unsignd)
IO_TO_LUA_VALUE_FUNC(uint32, uint32_t, LUA_VALUE_TYPE_UNSIGNED, unsignd)
IO_TO_LUA_VALUE_FUNC(uint64, uint64_t, LUA_VALUE_TYPE_UNSIGNED, unsignd)

IO_TO_LUA_VALUE_FUNC(float, float, LUA_VALUE_TYPE_NUMBER, number)
IO_TO_LUA_VALUE_FUNC(double, double, LUA_VALUE_TYPE_NUMBER, number)
IO_TO_LUA_VALUE_FUNC(longdouble, long double, LUA_VALUE_TYPE_NUMBER, number)

#define IO_PTR_TO_LUA_VALUE_FUNC(type_name, ctype, luatype, member) \
	static void io_##type_name##_to_lua_value(ctype data, io_lua_value_t *value) \
	{ \
		value->type = luatype; \
		value->value.member = data; \
	}

IO_PTR_TO_LUA_VALUE_FUNC(string, char *, LUA_VALUE_TYPE_STRING, string);
IO_PTR_TO_LUA_VALUE_FUNC(cfunction, lua_CFunction, LUA_VALUE_TYPE_CFUNCTION, cfunction);
IO_PTR_TO_LUA_VALUE_FUNC(list, void *, LUA_VALUE_TYPE_LIST, list);
IO_PTR_TO_LUA_VALUE_FUNC(table, void *, LUA_VALUE_TYPE_TABLE, table);

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

static void io_emb_register_free(emb_type_t *type, void *callback)
{
	io_emb_register_callback(type, "free", callback);
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

	type = emb_type_get("ushort");
	io_emb_register_to_lua_value(type, io_ushort_to_lua_value);

	type = emb_type_get("uint");
	io_emb_register_to_lua_value(type, io_uint_to_lua_value);

	type = emb_type_get("ulong");
	io_emb_register_to_lua_value(type, io_ulong_to_lua_value);

	type = emb_type_get("ulonglong");
	io_emb_register_to_lua_value(type, io_ulonglong_to_lua_value);

	type = emb_type_get("int8");
	io_emb_register_to_lua_value(type, io_int8_to_lua_value);

	type = emb_type_get("int16");
	io_emb_register_to_lua_value(type, io_int16_to_lua_value);

	type = emb_type_get("int32");
	io_emb_register_to_lua_value(type, io_int32_to_lua_value);

	type = emb_type_get("int64");
	io_emb_register_to_lua_value(type, io_int64_to_lua_value);

	type = emb_type_get("uint8");
	io_emb_register_to_lua_value(type, io_uint8_to_lua_value);

	type = emb_type_get("uint16");
	io_emb_register_to_lua_value(type, io_uint16_to_lua_value);

	type = emb_type_get("uint32");
	io_emb_register_to_lua_value(type, io_uint32_to_lua_value);

	type = emb_type_get("uint64");
	io_emb_register_to_lua_value(type, io_uint64_to_lua_value);

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
	io_emb_register_free(type, sdsfree);

	type = emb_type_get("string");
	io_emb_register_to_lua_value(type, io_string_to_lua_value);
	io_emb_register_free(type, free);
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
	io_emb_register_free(type, gds_slist_free);

	type = emb_type_get("gds_dlist");
	io_emb_register_to_lua_value(type, io_list_to_lua_value);
	io_emb_register_gds_iterator(type, gds_dlist_iterator_new);
	io_emb_register_free(type, gds_dlist_free);

	type = emb_type_get("gds_hash_map");
	io_emb_register_to_lua_value(type, io_table_to_lua_value);
	io_emb_register_gds_iterator(type, gds_hash_map_iterator_new);
	io_emb_register_free(type, gds_hash_map_free);

	type = emb_type_get("gds_hash_map_fast");
	io_emb_register_to_lua_value(type, io_table_to_lua_value);
	io_emb_register_gds_iterator(type, gds_hash_map_fast_iterator_new);
	io_emb_register_free(type, gds_hash_map_fast_free);

	type = emb_type_get("gds_hash_map_keyin");
	io_emb_register_to_lua_value(type, io_table_to_lua_value);
	io_emb_register_gds_iterator(type, gds_hash_map_keyin_iterator_new);
	io_emb_register_free(type, gds_hash_map_keyin_free);

	type = emb_type_get("gds_hash_map_keyin_fast");
	io_emb_register_to_lua_value(type, io_table_to_lua_value);
	io_emb_register_gds_iterator(type,
		gds_hash_map_keyin_fast_iterator_new);
	io_emb_register_free(type, gds_hash_map_keyin_fast_free);
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
