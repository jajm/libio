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

#include <stdio.h>
#include <stdlib.h>
#include <libgends/iterator.h>
#include <libgends/slist.h>
#include <libgends/hash_map.h>
#include "value.h"
#include "value_private.h"
#include "table_iterator.h"

struct io_table_iterator_s {
	gds_iterator_t *it;
	gds_slist_t *keys;
	io_value_t *v;
};

io_table_iterator_t * io_table_iterator(io_value_t *v)
{
	io_table_iterator_t *it;

	if (!io_value_isa(v, IO_VALUE_TABLE)) {
		fprintf(stderr, "Value v is not a table\n");
		return NULL;
	}

	it = malloc(sizeof(io_table_iterator_t));
	if (it == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		return NULL;
	}

	it->it = NULL;
	it->keys = NULL;
	it->v = v;

	io_table_iterator_reset(it);

	return it;
}

int io_table_iterator_reset(io_table_iterator_t *it)
{
	if (it == NULL)
		return -1;

	gds_iterator_free(it->it);
	gds_slist_free(it->keys, NULL, NULL);

	it->keys = gds_hash_map_keys(it->v->data.t);
	it->it = gds_slist_iterator_new(it->keys);

	return 0;
}

int io_table_iterator_step(io_table_iterator_t *it)
{
	if (it == NULL)
		return -1;
	
	return gds_iterator_step(it->it);
}

io_value_t * io_table_iterator_getkey(io_table_iterator_t *it)
{
	if (it == NULL)
		return NULL;
	
	return gds_iterator_get(it->it);
}

io_value_t * io_table_iterator_getvalue(io_table_iterator_t *it)
{
	io_value_t *k;

	if (it == NULL)
		return NULL;
	
	k = gds_iterator_get(it->it);

	return gds_hash_map_get(it->v->data.t, k);
}

void io_table_iterator_free(io_table_iterator_t *it)
{
	if (it != NULL) {
		gds_iterator_free(it->it);
		gds_slist_free(it->keys, NULL, NULL);
		free(it);
	}
}
