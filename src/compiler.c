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

enum io_compile_mode {
	IO_COMPILE_MODE_TEXT,
	IO_COMPILE_MODE_LUA,
	IO_COMPILE_MODE_LUA_EXPR,
	IO_COMPILE_MODE_STRING_SINGLE,
	IO_COMPILE_MODE_STRING_DOUBLE
};

char * io_compile(const char *template, const char *start_tag,
	const char *end_tag)
{
	sds buf = sdsnew("Io.output(\"");
	char cbuf[2] = "\0\0";
	char *out;
	const char *ptr = template;
	enum io_compile_mode mode = IO_COMPILE_MODE_TEXT;
	enum io_compile_mode previous_mode = IO_COMPILE_MODE_TEXT;
	size_t start_tag_len = strlen(start_tag);
	size_t end_tag_len = strlen(end_tag);
	size_t len;

	while (*ptr != '\0') {
		if (mode == IO_COMPILE_MODE_TEXT && *ptr == '\n') {
			buf = sdscat(buf, "\\n\");\nIo.output(\"");
		} else if (mode == IO_COMPILE_MODE_TEXT && *ptr == '"') {
			buf = sdscat(buf, "\\\"");
		} else if (mode == IO_COMPILE_MODE_TEXT && strncmp(ptr, start_tag, start_tag_len) == 0) {
			buf = sdscat(buf, "\");");
			if (ptr[start_tag_len] == '=') {
				mode = IO_COMPILE_MODE_LUA_EXPR;
				buf = sdscat(buf, "Io.output(");
				ptr += start_tag_len;
			} else {
				mode = IO_COMPILE_MODE_LUA;
				ptr += start_tag_len - 1;
			}
		} else if ((mode == IO_COMPILE_MODE_LUA || mode == IO_COMPILE_MODE_LUA_EXPR) && *ptr == '"') {
			previous_mode = mode;
			mode = IO_COMPILE_MODE_STRING_DOUBLE;
			buf = sdscat(buf, "\"");
		} else if (mode == IO_COMPILE_MODE_STRING_DOUBLE && *ptr == '"' && *(ptr-1) != '\\') {
			mode = previous_mode;
			buf = sdscat(buf, "\"");
		} else if ((mode == IO_COMPILE_MODE_LUA || mode == IO_COMPILE_MODE_LUA_EXPR) && *ptr == '\'') {
			previous_mode = mode;
			mode = IO_COMPILE_MODE_STRING_SINGLE;
			buf = sdscat(buf, "'");
		} else if (mode == IO_COMPILE_MODE_STRING_SINGLE && *ptr == '\'' && *(ptr-1) != '\\') {
			mode = previous_mode;
			buf = sdscat(buf, "'");
		} else if (mode == IO_COMPILE_MODE_LUA && strncmp(ptr, end_tag, end_tag_len) == 0) {
			buf = sdscat(buf, "Io.output(\"");
			mode = IO_COMPILE_MODE_TEXT;
			ptr += end_tag_len - 1;
		} else if (mode == IO_COMPILE_MODE_LUA_EXPR && strncmp(ptr, end_tag, end_tag_len) == 0) {
			buf = sdscat(buf, "); Io.output(\"");
			mode = IO_COMPILE_MODE_TEXT;
			ptr += end_tag_len - 1;
		} else {
			cbuf[0] = *ptr;
			buf = sdscat(buf, cbuf);
		}
		ptr++;
	}

	if (mode == IO_COMPILE_MODE_TEXT) {
		buf = sdscat(buf, "\")");
	} else if (mode == IO_COMPILE_MODE_LUA_EXPR) {
		buf = sdscat(buf, ")");
	}

	len = sdslen(buf);
	out = malloc(sizeof(char) * (len+1));
	strncpy(out, buf, len+1);
	sdsfree(buf);

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
