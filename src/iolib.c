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
#include <lua.h>
#include <lauxlib.h>
#include <sds.h>
#include "template.h"
#include "compiler.h"
#include "io_template_private.h"

static sds io_iolib_find_file(gds_slist_t *directories, const char *filename)
{
	sds filepath = NULL;
	sds d;
	FILE *fp;

	gds_slist_foreach(d, directories) {
		filepath = sdsdup(d);
		filepath = sdscat(filepath, "/");
		filepath = sdscat(filepath, filename);
		if ( (fp = fopen(filepath, "r")) ) {
			/* File exists and is readable */
			fclose(fp);
			break;
		}
		sdsfree(filepath);
	}

	return filepath;
}

int io_iolib_include(lua_State *L)
{
	const char *filename;
	sds filepath;
	io_template_t *T, *template;
	char *code;
	int n;
	lua_Debug ar;

	n = lua_gettop(L);

	lua_getfield(L, LUA_REGISTRYINDEX, "io_template");
	T = lua_touserdata(L, -1);
	lua_pop(L, 1);

	filename = lua_tostring(L, 1);
	filepath = io_iolib_find_file(T->config->directories, filename);
	if (filepath == NULL) {
		fprintf(stderr, "File %s not found\n", filename);
		return -1;
	}

	template = io_template_new(T->config);
	io_template_set_template_file(template, filepath);
	code = template->code;
	if (code != NULL) {
		int status = luaL_loadbuffer(L, code, strlen(code), filename);
		if (status == LUA_OK) {
			if (n > 1) {
				/* Set _ENV to given parameter. */
				lua_pushvalue(L, 2);
				lua_setupvalue(L, -2, 1);
			} else if (lua_getstack(L, 1, &ar) && lua_getinfo(L, "fn", &ar) && !lua_iscfunction(L, -1)) {
				/* Set _ENV = _ENV */
				lua_getupvalue(L, -1, 1);
				lua_setupvalue(L, -3, 1);
				lua_pop(L, 1);
			}
			if(lua_pcall(L, 0, 0, 0) != LUA_OK) {
				fprintf(stderr, "ERROR: %s\n", lua_tostring(L, -1));
			}
		} else {
			fprintf(stderr, "ERROR: %s\n", lua_tostring(L, -1));
		}
	}
	io_template_free(template);
	sdsfree(filepath);

	return 0;
}

int io_iolib_output(lua_State *L)
{
	sds *output_p;
	sds s;
	int i, n;

	n = lua_gettop(L);

	lua_getfield(L, LUA_REGISTRYINDEX, "io_output");
	output_p = lua_touserdata(L, -1);
	lua_pop(L, 1);

	for (i = -n; i < 0; i++) {
		switch (lua_type(L, i)) {
			case LUA_TBOOLEAN:
				s = sdsfromlonglong(lua_toboolean(L, i));
				break;
			case LUA_TNUMBER:
			case LUA_TSTRING:
				s = sdsnew(lua_tostring(L, i));
				break;

			default:
				s = sdsnew(lua_typename(L, lua_type(L, i)));
		}
		//fprintf(stderr, "DEBUG: %s (%s)\n", s, lua_typename(L, lua_type(L, i)));
		*output_p = sdscat(*output_p, s);
		sdsfree(s);
	}

	return 0;
}

static const char IO_IOLIB_NAME[] = "Io";
static const luaL_Reg io_iolib_functions[] = {
	{ "include", io_iolib_include },
	{ "output", io_iolib_output },
	{ NULL, NULL }
};

int io_luaopen_iolib(lua_State *L)
{
	luaL_newlib(L, io_iolib_functions);

	return 1;
}

int io_require_io(lua_State *L)
{
	luaL_requiref(L, IO_IOLIB_NAME, io_luaopen_iolib, 1);

	return 0;
}
