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
#include <sds.h>
#include "io_embody.h"

typedef struct {
	sds buffer;
	const char *start_tag;
	const char *end_tag;
	size_t start_tag_len;
	size_t end_tag_len;
	unsigned int multiline_equal_signs_count;
} io_compile_context_t;

static const char * io_compile_string_single(const char *ptr,
	io_compile_context_t *context)
{
	sds *buf = &(context->buffer);

	while (*ptr) {
		if (*ptr == '\'' && *(ptr - 1) != '\\') {
			*buf = sdscat(*buf, "'");
			break;
		} else {
			*buf = sdscatlen(*buf, ptr, 1);
		}
		ptr++;
	}

	return ptr;
}

static const char * io_compile_string_double(const char *ptr,
	io_compile_context_t *context)
{
	sds *buf = &(context->buffer);

	while (*ptr) {
		if (*ptr == '"' && *(ptr - 1) != '\\') {
			*buf = sdscat(*buf, "\"");
			break;
		} else {
			*buf = sdscatlen(*buf, ptr, 1);
		}
		ptr++;
	}

	return ptr;
}

static const char * io_compile_string_multiline(const char *ptr,
	io_compile_context_t *context)
{
	sds *buf = &(context->buffer);
	unsigned int n_equals = context->multiline_equal_signs_count;
	unsigned int n_equals_count;

	while (*ptr) {
		if (*ptr == ']') {
			n_equals_count = 0;
			while (*(ptr + n_equals_count + 1) == '=') {
				n_equals_count++;
			}

			if (n_equals_count == n_equals && *(ptr + n_equals + 1) == ']') {
				*buf = sdscatlen(*buf, ptr, n_equals + 2);
				ptr += n_equals + 1;
				break;
			} else {
				*buf = sdscat(*buf, "]");
			}
		} else {
			*buf = sdscatlen(*buf, ptr, 1);
		}
		ptr++;
	}

	return ptr;
}

static const char * io_compile_lua(const char *ptr,
	io_compile_context_t *context)
{
	sds *buf = &(context->buffer);
	const char *end_tag = context->end_tag;
	size_t end_tag_len = context->end_tag_len;
	int is_lua_expr = 0;

	if (*ptr == '=') {
		is_lua_expr = 1;
		*buf = sdscat(*buf, "Io.output(");
		ptr++;
	}

	while (*ptr) {
		if (*ptr == '\'') {
			*buf = sdscat(*buf, "'");
			ptr = io_compile_string_single(ptr + 1, context);
		} else if (*ptr == '"') {
			*buf = sdscat(*buf, "\"");
			ptr = io_compile_string_double(ptr + 1, context);
		} else if (*ptr == '[') {
			int n_equals = 0;
			while (*(ptr + n_equals + 1) == '=') n_equals++;
			if (*(ptr + n_equals + 1) == '[') {
				*buf = sdscatlen(*buf, ptr, n_equals + 2);
				context->multiline_equal_signs_count = n_equals;
				ptr = io_compile_string_multiline(
					ptr + n_equals + 2, context);
			} else {
				*buf = sdscat(*buf, "[");
			}
		} else if (!strncmp(ptr, end_tag, end_tag_len)) {
			ptr += end_tag_len - 1;
			break;
		} else {
			*buf = sdscatlen(*buf, ptr, 1);
		}
		ptr++;
	}

	if (is_lua_expr) {
		*buf = sdscat(*buf, ");");
	}

	return ptr;
}

static const char * io_compile_text(const char *ptr,
	io_compile_context_t *context)
{
	sds *buf = &(context->buffer);
	const char *start_tag = context->start_tag;
	size_t start_tag_len = context->start_tag_len;

	*buf = sdscat(*buf, "Io.output(\"");
	while (*ptr) {
		if (*ptr == '\n') {
			*buf = sdscat(*buf, "\\n\");\nIo.output(\"");
		} else if (*ptr == '"') {
			*buf = sdscat(*buf, "\\\"");
		} else if (!strncmp(ptr, start_tag, start_tag_len)) {
			*buf = sdscat(*buf, "\");");
			ptr = io_compile_lua(ptr + start_tag_len, context);
			*buf = sdscat(*buf, "Io.output(\"");
		} else {
			*buf = sdscatlen(*buf, ptr, 1);
		}
		ptr++;
	}
	*buf = sdscat(*buf, "\");");

	return ptr;
}

char * io_compile(const char *template, const char *start_tag,
	const char *end_tag)
{
	char *out;
	const char *ptr = template;
	size_t len;

	io_compile_context_t context;
	context.buffer = sdsempty(),
	context.start_tag = start_tag;
	context.end_tag = end_tag;
	context.start_tag_len = strlen(start_tag);
	context.end_tag_len = strlen(end_tag);
	context.multiline_equal_signs_count = 0;

	io_compile_text(ptr, &context);

	len = sdslen(context.buffer);
	out = malloc(sizeof(char) * (len+1));
	strncpy(out, context.buffer, len+1);
	sdsfree(context.buffer);

	return out;
}

char * io_compile_filep(FILE *filep, const char *start_tag, const char *end_tag)
{
	char buf[1024];
	char *out;
	sds tpl;

	if (filep == NULL) {
		fprintf(stderr, "filep is NULL\n");
		return NULL;
	}

	tpl = sdsempty();

	while (!feof(filep)) {
		if(fgets(buf, 1024, filep)) {
			tpl = sdscat(tpl, buf);
		}
	}

	out = io_compile(tpl, start_tag, end_tag);
	sdsfree(tpl);

	return out;
}

char * io_compile_file(const char *filename, const char *start_tag,
	const char *end_tag)
{
	FILE *fp;
	char *out = NULL;

	fp = fopen(filename, "r");
	if (fp != NULL) {
		out = io_compile_filep(fp, start_tag, end_tag);
		fclose(fp);
	}

	return out;
}
