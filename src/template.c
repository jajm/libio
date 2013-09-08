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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <object/string.h>
#include "iolib.h"
#include "value.h"
#include "table_iterator.h"
#include "compiler.h"
#include "template.h"

struct io_template_s {
	_Bool data_is_a_filename;
	char *data;
	io_value_t *stash;
};

io_template_t * io_template_new(const char *template)
{
	io_template_t *T = NULL;
	size_t len;

	if (template == NULL || template[0] == '\0') {
		fprintf(stderr, "Template is empty in io_template_new\n");
		return NULL;
	}

	T = malloc(sizeof(io_template_t));
	if (T == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		return NULL;
	}

	len = strlen(template);
	T->data = malloc(sizeof(char) * (len+1));
	if (T->data == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		free(T);
		return NULL;
	}
	strncpy(T->data, template, len+1);
	T->data_is_a_filename = false;

	T->stash = io_value_table();

	return T;
}

io_template_t * io_template_new_from_file(const char *filename)
{
	io_template_t *T;

	T = io_template_new(filename);
	if (T != NULL) {
		T->data_is_a_filename = true;
	}

	return T;
}

void io_template_param(io_template_t *T, const char *name, io_value_t *value)
{
	if (T != NULL) {
		io_value_t *key = io_value_string(name);
		io_value_table_set(T->stash, key, value);
	} else {
		fprintf(stderr, "T is NULL in io_template_param\n");
	}
}

void io_value_to_lua_stack(io_value_t *v, lua_State *L)
{
	io_table_iterator_t *it;
	io_value_t *k, *val;

	switch (io_value_type(v)) {
		case IO_VALUE_NIL:
			lua_pushnil(L);
			break;

		case IO_VALUE_BOOLEAN:
			lua_pushboolean(L, io_value_boolean_get(v));
			break;

		case IO_VALUE_INTEGER:
			lua_pushinteger(L, io_value_integer_get(v));
			break;

		case IO_VALUE_NUMBER:
			lua_pushnumber(L, io_value_number_get(v));
			break;

		case IO_VALUE_STRING:
			lua_pushstring(L, io_value_string_get(v));
			break;

		case IO_VALUE_CFUNCTION:
			lua_pushcfunction(L, io_value_cfunction_get(v));
			break;

		case IO_VALUE_TABLE:
			lua_newtable(L);
			it = io_table_iterator(v);
			while (!io_table_iterator_step(it)) {
				k = io_table_iterator_getkey(it);
				val = io_table_iterator_getvalue(it);
				io_value_to_lua_stack(k, L);
				io_value_to_lua_stack(val, L);
				lua_settable(L, -3);
			}
			io_table_iterator_free(it);
			break;
		default:
			fprintf(stderr, "Unknown type\n");
	}
}

char * io_template_render(io_template_t *T)
{
	char *out = NULL;
	lua_State *L;
	size_t len;
	char *lua_code, *lua_name;
	static const char *lua_main_chunk =
		"setmetatable(stash, { __index = _ENV })\n"
		"f = assert(load(code, name, 't', stash))\n"
		"f()";

	L = luaL_newstate();

	luaL_openlibs(L);
	io_require_io(L);

	string_t *output = string("");
	lua_pushlightuserdata(L, output);
	lua_setfield(L, LUA_REGISTRYINDEX, "io_output");

	io_value_to_lua_stack(T->stash, L);
	lua_setglobal(L, "stash");

	if (T->data_is_a_filename) {
		lua_code = io_compile_file(T->data, "#{", "}#");
		lua_name = T->data;
	} else {
		lua_code = io_compile(T->data, "#{", "}#");
		lua_name = "=(Io:main)";
	}

	lua_pushstring(L, lua_code);
	lua_setglobal(L, "code");
	lua_pushstring(L, lua_name);
	lua_setglobal(L, "name");

	if (luaL_dostring(L, lua_main_chunk) != 0) {
		fprintf(stderr, "error: %s\n", lua_tostring(L, -1));
		lua_pop(L, -1);
	}

	len = string_length(output);
	out = malloc(sizeof(char) * (len+1));
	strncpy(out, string_to_c_str(output), len+1);
	string_free(output);

	lua_close(L);

	return out;
}

void io_template_free(io_template_t *T)
{
	if (T != NULL) {
		free(T->data);
		io_value_free(T->stash);
		free(T);
	}
}

