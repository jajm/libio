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

#ifndef io_lua_value_h_included
#define io_lua_value_h_included

#include <lua.h>

typedef struct {
	enum {
		LUA_VALUE_TYPE_NONE = 0,
		LUA_VALUE_TYPE_NIL,
		LUA_VALUE_TYPE_BOOLEAN,
		LUA_VALUE_TYPE_INTEGER,
		LUA_VALUE_TYPE_UNSIGNED,
		LUA_VALUE_TYPE_NUMBER,
		LUA_VALUE_TYPE_STRING,
		LUA_VALUE_TYPE_CFUNCTION,
		LUA_VALUE_TYPE_LIGHTUSERDATA
	} type;
	union {
		int boolean;
		lua_Integer integer;
		lua_Unsigned unsignd;
		lua_Number number;
		const char *string;
		lua_CFunction cfunction;
		void *lightuserdata;
	} value;
} io_lua_value_t;


#endif /* ! io_lua_value_h_included */
