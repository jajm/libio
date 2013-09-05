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
#include <libgends/hash_map.h>
#include <lua.h>
#include "value.h"
#include "value_private.h"

io_value_t * io_value_new(io_value_type_t type)
{
	io_value_t *v;

	v = malloc(sizeof(io_value_t));
	if (v == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		return NULL;
	}

	v->type = type;

	return v;
}

io_value_t * io_value_nil(void)
{
	io_value_t *v;

	v = io_value_new(IO_VALUE_NIL);

	return v;
}

io_value_t * io_value_boolean(int b)
{
	io_value_t *v;

	v = io_value_new(IO_VALUE_BOOLEAN);
	if (v != NULL) {
		v->data.b = b;
	}

	return v;
}

int io_value_boolean_set(io_value_t *v, int b)
{
	if (!io_value_isa(v, IO_VALUE_BOOLEAN)) {
		return -1;
	}

	v->data.b = b;

	return 0;
}

int io_value_boolean_get(const io_value_t *v)
{
	int b = 0;

	if (io_value_isa(v, IO_VALUE_BOOLEAN)) {
		b = v->data.b;
	}

	return b;
}

io_value_t * io_value_integer(lua_Integer i)
{
	io_value_t *v;

	v = io_value_new(IO_VALUE_INTEGER);
	if (v != NULL) {
		v->data.i = i;
	}

	return v;
}

int io_value_integer_set(io_value_t *v, lua_Integer i)
{
	if (!io_value_isa(v, IO_VALUE_INTEGER)) {
		return -1;
	}

	v->data.i = i;

	return 0;
}

lua_Integer io_value_integer_get(const io_value_t *v)
{
	lua_Integer i = 0;

	if (io_value_isa(v, IO_VALUE_INTEGER)) {
		i = v->data.i;
	}

	return i;
}

io_value_t * io_value_number(lua_Number n)
{
	io_value_t *v;

	v = io_value_new(IO_VALUE_NUMBER);
	if (v != NULL) {
		v->data.n = n;
	}

	return v;
}

int io_value_number_set(io_value_t *v, lua_Number n)
{
	if (!io_value_isa(v, IO_VALUE_NUMBER)) {
		return -1;
	}

	v->data.n = n;

	return 0;
}

lua_Number io_value_number_get(const io_value_t *v)
{
	lua_Number n = 0;

	if (io_value_isa(v, IO_VALUE_NUMBER)) {
		n = v->data.n;
	}

	return n;
}

io_value_t * io_value_string(const char *s)
{
	io_value_t *v;
	size_t len;

	v = io_value_new(IO_VALUE_STRING);
	if (v != NULL) {
		len = strlen(s);
		v->data.s = malloc(sizeof(char) * (len+1));
		if (v->data.s == NULL) {
			fprintf(stderr, "Memory allocation error\n");
			free(v);
			return NULL;
		}
		strncpy(v->data.s, s, len+1);
	}

	return v;
}

int io_value_string_set(io_value_t *v, const char *s)
{
	size_t len;
	char *buf;

	if (!io_value_isa(v, IO_VALUE_STRING)) {
		return -1;
	}

	len = strlen(s);
	buf = malloc(sizeof(char) * (len+1));
	if (buf == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		return -1;
	}
	strncpy(buf, s, len+1);
	free(v->data.s);
	v->data.s = buf;

	return 0;
}

const char * io_value_string_get(const io_value_t *v)
{
	const char *s = NULL;

	if (io_value_isa(v, IO_VALUE_STRING)) {
		s = v->data.s;
	}

	return s;
}

io_value_t * io_value_cfunction(lua_CFunction f)
{
	io_value_t *v;

	v = io_value_new(IO_VALUE_CFUNCTION);
	if (v != NULL) {
		v->data.f = f;
	}

	return v;
}

int io_value_cfunction_set(io_value_t *v, lua_CFunction f)
{
	if (!io_value_isa(v, IO_VALUE_CFUNCTION)) {
		return -1;
	}

	v->data.f = f;

	return 0;
}

lua_CFunction io_value_cfunction_get(const io_value_t *v)
{
	lua_CFunction f = NULL;

	if (io_value_isa(v, IO_VALUE_CFUNCTION)) {
		f = v->data.f;
	}

	return f;
}

static const unsigned int IO_VALUE_TABLE_HASH_SIZE = 128;

unsigned int io_value_table_string_hash(const char *s)
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

unsigned int io_value_table_hash_callback(const io_value_t *v, unsigned int size)
{
	unsigned int hash = 0;

	if (v != NULL) {
		const char *s;
		switch (io_value_type(v)) {
			case IO_VALUE_BOOLEAN:
				hash = (unsigned int) (io_value_boolean_get(v) ? 1 : 0);
				break;
			case IO_VALUE_INTEGER:
				hash = (unsigned int) io_value_integer_get(v);
				break;
			case IO_VALUE_NUMBER:
				hash = (unsigned int) io_value_number_get(v);
				break;
			case IO_VALUE_STRING:
				s = io_value_string_get(v);
				hash = io_value_table_string_hash(s);
				break;
			case IO_VALUE_CFUNCTION:
				hash = (intptr_t) io_value_cfunction_get(v);
				break;
			case IO_VALUE_TABLE:
				hash = (intptr_t) v->data.t;
				break;
			default:
				fprintf(stderr, "Unknown type\n");
		}
	}

	return hash % size;
}

int io_value_table_cmpkey_callback(const io_value_t *v1, const io_value_t *v2)
{
	int cmp = 0;

	if (v1 == NULL && v2 == NULL) {
		cmp = 0;
	} else if (v1 == NULL && v2 != NULL) {
		cmp = 1;
	} else if (v1 != NULL && v2 == NULL) {
		cmp = -1;
	} else if (io_value_type(v1) != io_value_type(v2)) {
		cmp = io_value_type(v1) - io_value_type(v2);
	} else {
		/* v1 and v2 are of the same type */
		int b1, b2;
		const char *s1, *s2;
		switch (io_value_type(v1)) {
			case IO_VALUE_BOOLEAN:
				b1 = io_value_boolean_get(v1) ? 1 : 0;
				b2 = io_value_boolean_get(v2) ? 1 : 0;
				cmp = b1 - b2;
				break;
			case IO_VALUE_INTEGER:
				cmp = io_value_integer_get(v1) - io_value_integer_get(v2);
				break;
			case IO_VALUE_NUMBER:
				cmp = (int) (io_value_number_get(v1) - io_value_number_get(v2));
				break;
			case IO_VALUE_STRING:
				s1 = io_value_string_get(v1);
				s2 = io_value_string_get(v2);
				cmp = strcmp(s1, s2);
				break;
			case IO_VALUE_CFUNCTION:
				cmp = (int) (io_value_cfunction_get(v1) - io_value_cfunction_get(v2));
				break;
			case IO_VALUE_TABLE:
				cmp = (int) (v1->data.t - v2->data.t);
				break;
			default:
				fprintf(stderr, "Unknown type\n");
		}
	}

	return cmp;
}

io_value_t * io_value_table(void)
{
	io_value_t *v;

	v = io_value_new(IO_VALUE_TABLE);
	if (v != NULL) {
		v->data.t = gds_hash_map_new(IO_VALUE_TABLE_HASH_SIZE,
			(gds_hash_cb) io_value_table_hash_callback,
			(gds_cmpkey_cb) io_value_table_cmpkey_callback);
	}

	return v;
}

int io_value_table_set(io_value_t *v, io_value_t *k, io_value_t *val)
{
	if (io_value_isa(v, IO_VALUE_TABLE)) {
		gds_hash_map_set(v->data.t, k, val, (gds_free_cb)io_value_free);
	}

	return 0;
}

io_value_t * io_value_table_get(const io_value_t *v, io_value_t *k)
{
	io_value_t *val = NULL;

	if (io_value_isa(v, IO_VALUE_TABLE)) {
		val = gds_hash_map_get(v->data.t, k);
	}

	return val;
}

int io_value_isa(const io_value_t *v, io_value_type_t t)
{
	if (v != NULL && v->type == t) {
		return 1;
	}

	return 0;
}

io_value_type_t io_value_type(const io_value_t *v)
{
	if (v != NULL) {
		return v->type;
	}

	return -1;
}

io_value_t * io_value_copy(io_value_t *v)
{
	io_value_t *copy = NULL;

	if (v != NULL) {
		size_t len;
		gds_slist_t *keys;
		io_value_t *k;
		copy = io_value_new(v->type);
		switch (copy->type) {
			case IO_VALUE_NIL:
				break;

			case IO_VALUE_BOOLEAN:
				copy->data.b = v->data.b;
				break;

			case IO_VALUE_INTEGER:
				copy->data.i = v->data.i;
				break;

			case IO_VALUE_NUMBER:
				copy->data.n = v->data.n;
				break;

			case IO_VALUE_STRING:
				len = strlen(v->data.s);
				copy->data.s = malloc(sizeof(char) * (len+1));
				strncpy(copy->data.s, v->data.s, len+1);
				break;

			case IO_VALUE_CFUNCTION:
				copy->data.f = v->data.f;
				break;

			case IO_VALUE_TABLE:
				keys = gds_hash_map_keys(v->data.t);
				copy->data.t = gds_hash_map_new(
					IO_VALUE_TABLE_HASH_SIZE,
					(gds_hash_cb) io_value_table_hash_callback,
					(gds_cmpkey_cb) io_value_table_cmpkey_callback);
				gds_slist_foreach(k, keys) {
					io_value_t *val = gds_hash_map_get(v->data.t, k);
					io_value_table_set(copy, io_value_copy(k), io_value_copy(val));
				}
				gds_slist_free(keys, NULL, NULL);
				break;
		}
	}

	return copy;
}

void io_value_free(io_value_t *v)
{
	if (v != NULL) {
		switch (v->type) {
			case IO_VALUE_STRING:
				free(v->data.s);
				break;

			case IO_VALUE_TABLE:
				gds_hash_map_free(v->data.t,
					(gds_free_cb) io_value_free,
					(gds_free_cb) io_value_free);
				break;

			case IO_VALUE_NIL:
			case IO_VALUE_BOOLEAN:
			case IO_VALUE_INTEGER:
			case IO_VALUE_NUMBER:
			case IO_VALUE_CFUNCTION:
			default:
				/* do nothing */
				;

		}
		free(v);
	}
}
