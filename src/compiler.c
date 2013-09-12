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
#include <libobject/string.h>

enum io_compile_mode {
	IO_COMPILE_MODE_TEXT,
	IO_COMPILE_MODE_LUA,
	IO_COMPILE_MODE_LUA_EXPR
};

char * io_compile(const char *template, const char *start_tag,
	const char *end_tag)
{
	string_t *buf = string("Io.output(\"");
	char cbuf[2] = "\0\0";
	char *out;
	const char *ptr = template;
	enum io_compile_mode mode = IO_COMPILE_MODE_TEXT;
	size_t start_tag_len = strlen(start_tag);
	size_t end_tag_len = strlen(end_tag);
	size_t len;
	const char *tmp;

	while (*ptr != '\0') {
		if (mode == IO_COMPILE_MODE_TEXT && *ptr == '\n') {
			string_cat(buf, "\\n\");\nIo.output(\"");
		} else if (mode == IO_COMPILE_MODE_TEXT && *ptr == '"') {
			string_cat(buf, "\\\"");
		} else if (mode == IO_COMPILE_MODE_TEXT && strncmp(ptr, start_tag, start_tag_len) == 0) {
			string_cat(buf, "\");");
			if (ptr[start_tag_len] == '=') {
				mode = IO_COMPILE_MODE_LUA_EXPR;
				string_cat(buf, "Io.output(");
				ptr += start_tag_len;
			} else {
				mode = IO_COMPILE_MODE_LUA;
				ptr += start_tag_len - 1;
			}
		} else if (mode == IO_COMPILE_MODE_LUA && strncmp(ptr, end_tag, end_tag_len) == 0) {
			string_cat(buf, "Io.output(\"");
			mode = IO_COMPILE_MODE_TEXT;
			ptr += end_tag_len - 1;
		} else if (mode == IO_COMPILE_MODE_LUA_EXPR && strncmp(ptr, end_tag, end_tag_len) == 0) {
			string_cat(buf, "); Io.output(\"");
			mode = IO_COMPILE_MODE_TEXT;
			ptr += end_tag_len - 1;
		} else {
			cbuf[0] = *ptr;
			string_cat(buf, cbuf);
		}
		ptr++;
	}

	if (mode == IO_COMPILE_MODE_TEXT) {
		string_cat(buf, "\")");
	} else if (mode == IO_COMPILE_MODE_LUA_EXPR) {
		string_cat(buf, ")");
	}

	tmp = string_to_c_str(buf);
	len = strlen(tmp);
	out = malloc(sizeof(char) * (len+1));
	strncpy(out, tmp, len+1);
	string_free(buf);

	return out;
}

char * io_compile_filep(FILE *filep, const char *start_tag, const char *end_tag)
{
	char buf[1024];
	char *out;
	string_t *tpl;

	if (filep == NULL) {
		fprintf(stderr, "filep is NULL\n");
		return NULL;
	}

	tpl = string("");

	while (!feof(filep)) {
		if(fgets(buf, 1024, filep)) {
			string_cat(tpl, buf);
		}
	}

	out = io_compile(string_to_c_str(tpl), start_tag, end_tag);
	string_free(tpl);

	return out;
}

char * io_compile_file(const char *filename, const char *start_tag,
	const char *end_tag)
{
	FILE *fp;
	char *out;

	fp = fopen(filename, "r");
	out = io_compile_filep(fp, start_tag, end_tag);
	fclose(fp);

	return out;
}
