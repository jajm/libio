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
#include "io_config.h"

static const char io_default_code_start_tag[] = "{%";
static const char io_default_code_end_tag[] = "%}";
static const char io_default_expr_start_tag[] = "{{";
static const char io_default_expr_end_tag[] = "}}";
static const char io_default_comm_start_tag[] = "{#";
static const char io_default_comm_end_tag[] = "#}";

io_config_t * io_config_new(const char *code_start_tag,
	const char *code_end_tag, const char *expr_start_tag,
	const char *expr_end_tag, const char *comm_start_tag,
	const char *comm_end_tag)
{
	io_config_t *config;

	config = malloc(sizeof(io_config_t));

	/* Set defaults */
	if (!code_start_tag) code_start_tag = io_default_code_start_tag;
	if (!code_end_tag) code_end_tag = io_default_code_end_tag;
	if (!expr_start_tag) expr_start_tag = io_default_expr_start_tag;
	if (!expr_end_tag) expr_end_tag = io_default_expr_end_tag;
	if (!comm_start_tag) comm_start_tag = io_default_comm_start_tag;
	if (!comm_end_tag) comm_end_tag = io_default_comm_end_tag;

	config->code_start_tag = sdsnew(code_start_tag);
	config->code_end_tag = sdsnew(code_end_tag);
	config->expr_start_tag = sdsnew(expr_start_tag);
	config->expr_end_tag = sdsnew(expr_end_tag);
	config->comm_start_tag = sdsnew(comm_start_tag);
	config->comm_end_tag = sdsnew(comm_end_tag);

	config->directories = gds_slist_new(sdsfree);
	gds_slist_push(config->directories, sdsnew("."));

	return config;
}

io_config_t * io_config_new_default(void)
{
	return io_config_new(NULL, NULL, NULL, NULL, NULL, NULL);
}

void io_config_free(io_config_t *config)
{
	if (config) {
		sdsfree(config->code_start_tag);
		sdsfree(config->code_end_tag);
		sdsfree(config->expr_start_tag);
		sdsfree(config->expr_end_tag);
		sdsfree(config->comm_start_tag);
		sdsfree(config->comm_end_tag);

		gds_slist_free(config->directories);

		free(config);
	}
}
