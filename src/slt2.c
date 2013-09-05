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
#include <lua.h>
#include "slt2.lua.h"

int io_require_slt2(lua_State *L)
{
	int ret;

	ret = lua2c_do_slt2_lua(L);
	if (!ret) {
		lua_setglobal(L, "slt2");
	} else {
		fprintf(stderr, "Error while loading or executing slt2.lua precompiled chunk: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	return ret;
}
