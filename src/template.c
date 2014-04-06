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
#include <sds.h>
#include <embody/embody.h>
#include <libgends/hash_map.h>
#include <libgends/hash_functions.h>
#include "iolib.h"
#include "compiler.h"
#include "template.h"
#include "io_embody.h"
#include "lua_value.h"

struct io_template_s {
	_Bool data_is_a_filename;
	char *data;
	void **stash;
	char *last_render;
};

static int io_initialized = 0;
void io_initialize(void)
{
	if (!io_initialized) {
		io_emb_initialize();

		io_initialized = 1;
	}
}

io_template_t * io_template_new(const char *template)
{
	io_template_t *T = NULL;
	size_t len;

	io_initialize();

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
	T->last_render = NULL;

	gds_hash_map_t *stash = gds_hash_map_new(128, gds_hash_djb2, strcmp,
		NULL, emb_container_free, emb_container_free);
	T->stash = emb_new("gds_hash_map", stash);

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

void io_template_param(io_template_t *T, const char *name, void *value)
{
	if (T != NULL) {
		gds_hash_map_t *stash_p = *(T->stash);
		gds_hash_map_set(stash_p, emb_new("sds", sdsnew(name)), value);
	} else {
		fprintf(stderr, "T is NULL in io_template_param\n");
	}
}

void io_object_to_lua_stack(void **object, lua_State *L);

static void io_list_to_lua_stack(void **list, lua_State *L)
{
	gds_iterator_t *(*iterator_callback)(void *);
	emb_type_t *type;

	type = emb_type(list);
	iterator_callback = emb_type_get_callback(type, "gds_iterator");
	if (iterator_callback) {
		gds_iterator_t *it;
		void **val;
		unsigned int i = 1;

		lua_newtable(L);
		it = iterator_callback(*list);
		while (!gds_iterator_step(it)) {
			val = gds_iterator_get(it);
			lua_pushunsigned(L, i);
			io_object_to_lua_stack(val, L);
			lua_settable(L, -3);
			i++;
		}
		gds_iterator_free(it);
	} else {
		lua_pushnil(L);
	}
}

static void io_table_to_lua_stack(void **table, lua_State *L)
{
	gds_iterator_t *(*iterator_callback)(void *);
	emb_type_t *type;

	type = emb_type(table);
	iterator_callback = emb_type_get_callback(type, "gds_iterator");
	if (iterator_callback) {
		gds_iterator_t *it;
		void **k, **val;

		lua_newtable(L);
		it = iterator_callback(*table);
		while (!gds_iterator_step(it)) {
			k = gds_iterator_getkey(it);
			val = gds_iterator_get(it);
			io_object_to_lua_stack(k, L);
			io_object_to_lua_stack(val, L);
			lua_settable(L, -3);
		}
		gds_iterator_free(it);
	} else {
		lua_pushnil(L);
	}
}

void io_object_to_lua_stack(void **object, lua_State *L)
{
	if (object == NULL) {
		lua_pushnil(L);
		return;
	}

	io_lua_value_t lua_value;
	lua_value.type = LUA_VALUE_TYPE_NONE;
	io_emb_data_to_lua_value(object, &lua_value);
	if (lua_value.type != LUA_VALUE_TYPE_NONE) {
		switch (lua_value.type) {
			case LUA_VALUE_TYPE_NIL:
				lua_pushnil(L);
				break;
			case LUA_VALUE_TYPE_BOOLEAN:
				lua_pushboolean(L, lua_value.value.boolean);
				break;
			case LUA_VALUE_TYPE_INTEGER:
				lua_pushinteger(L, lua_value.value.integer);
				break;
			case LUA_VALUE_TYPE_UNSIGNED:
				lua_pushunsigned(L, lua_value.value.unsignd);
				break;
			case LUA_VALUE_TYPE_NUMBER:
				lua_pushnumber(L, lua_value.value.number);
				break;
			case LUA_VALUE_TYPE_STRING:
				lua_pushstring(L, lua_value.value.string);
				break;
			case LUA_VALUE_TYPE_CFUNCTION:
				lua_pushcfunction(L, lua_value.value.cfunction);
				break;
			case LUA_VALUE_TYPE_LIST:
				io_list_to_lua_stack(object, L);
				break;
			case LUA_VALUE_TYPE_TABLE:
				io_table_to_lua_stack(object, L);
				break;
			case LUA_VALUE_TYPE_LIGHTUSERDATA:
				lua_pushlightuserdata(L, lua_value.value.lightuserdata);
				break;
			default:
				lua_pushnil(L);
		}
	} else {
		fprintf(stderr, "Unknown type (%s) in %s\n",
			emb_type_name(object), __func__);
		lua_pushnil(L);
	}
}

const char * io_template_render(io_template_t *T)
{
	lua_State *L;
	size_t len;
	char *lua_code, *lua_name;

	L = luaL_newstate();

	luaL_openlibs(L);
	io_require_io(L);

	sds output = sdsempty();
	lua_pushlightuserdata(L, &output);
	lua_setfield(L, LUA_REGISTRYINDEX, "io_output");

	if (T->data_is_a_filename) {
		lua_code = io_compile_file(T->data, "#{", "}#");
		lua_name = T->data;
	} else {
		lua_code = io_compile(T->data, "#{", "}#");
		lua_name = "=(Io:main)";
	}

	if (luaL_loadbuffer(L, lua_code, strlen(lua_code), lua_name) == LUA_OK) {
		// stash = ...
		io_object_to_lua_stack(T->stash, L);

		// stash_mt = { __index = _G }
		lua_newtable(L);
		lua_pushinteger(L, LUA_RIDX_GLOBALS);
		lua_gettable(L, LUA_REGISTRYINDEX);
		lua_setfield(L, -2, "__index");

		// setmetatable(stash, stash_mt)
		lua_setmetatable(L, -2);

		// Set environment and call function.
		lua_setupvalue(L, -2, 1);
		lua_pcall(L, 0, 0, 0);
	} else {
		fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
	}

	free(lua_code);

	len = sdslen(output);
	free(T->last_render);
	T->last_render = malloc(sizeof(char) * (len+1));
	strncpy(T->last_render, output, len+1);
	sdsfree(output);

	lua_close(L);

	return T->last_render;
}

void io_template_free(io_template_t *T)
{
	if (T != NULL) {
		free(T->data);
		emb_free(T->stash);
		free(T->last_render);
		free(T);
	}
}

