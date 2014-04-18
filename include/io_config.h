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

#ifndef io_config_h_included
#define io_config_h_included

#include <sds.h>
#include <libgends/slist.h>

typedef struct {
	sds code_start_tag;
	sds code_end_tag;
	sds expr_start_tag;
	sds expr_end_tag;
	sds comm_start_tag;
	sds comm_end_tag;

	gds_slist_t *directories;
} io_config_t;

io_config_t *
io_config_new(
	const char *code_start_tag,
	const char *code_end_tag,
	const char *expr_start_tag,
	const char *expr_end_tag,
	const char *comm_start_tag,
	const char *comm_end_tag
);

io_config_t *
io_config_new_default(void);

void
io_config_free(
	io_config_t *config
);

#endif /* ! io_config_h_included */
