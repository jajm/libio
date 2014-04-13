/*
 * Copyright 2013-2014 Julian Maurice
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
#include <libgends/dlist.h>

typedef struct {
	gds_dlist_t *tokens;
	const char *start_tag;
	const char *end_tag;
	size_t start_tag_len;
	size_t end_tag_len;
} io_parser_context_t;

typedef enum {
	IO_TOKEN_TYPE_PLAIN,
	IO_TOKEN_TYPE_TEXT,
	IO_TOKEN_TYPE_WHITESPACE,
	IO_TOKEN_TYPE_NEWLINE,
	IO_TOKEN_TYPE_LUA_EXPR
} io_token_type_t;

typedef struct {
	io_token_type_t type;
	sds value;
} io_token_t;

static io_token_t * io_token_new(io_token_type_t type, sds value)
{
	io_token_t *token;

	token = malloc(sizeof(io_token_t));
	token->type = type;
	token->value = value;

	return token;
}

static void io_token_free(io_token_t *token)
{
	if (token) {
		sdsfree(token->value);
		free(token);
	}
}

static const char * io_parser_parse_string_single(const char *ptr)
{
	ptr = strchr(ptr + 1, '\'');
	while (ptr && *(ptr - 1) == '\\') {
		ptr = strchr(ptr + 1, '\'');
	}

	return ptr;
}

static const char * io_parser_parse_string_double(const char *ptr)
{
	ptr = strchr(ptr + 1, '"');
	while (ptr && *(ptr - 1) == '\\') {
		ptr = strchr(ptr + 1, '"');
	}

	return ptr;
}

static const char * io_parser_parse_string_multiline(const char *ptr,
	unsigned int n_equals)
{
	char multi_end_tag[n_equals + 3];

	multi_end_tag[0] = ']';
	memset(multi_end_tag + 1, '=', n_equals);
	multi_end_tag[n_equals + 1] = ']';
	multi_end_tag[n_equals + 2] = '\0';

	ptr = strstr(ptr, multi_end_tag);
	if (ptr) {
		ptr += n_equals + 1;
	}

	return ptr;
}

static const char * io_parser_parse_lua(const char *ptr,
	io_parser_context_t *context)
{
	gds_dlist_t *tokens = context->tokens;
	const char *end_tag = context->end_tag;
	size_t end_tag_len = context->end_tag_len;
	const char *tmp;
	io_token_type_t type = IO_TOKEN_TYPE_PLAIN;
	sds value;

	ptr += context->start_tag_len;
	if (*ptr == '=') {
		type = IO_TOKEN_TYPE_LUA_EXPR;
		ptr++;
	}
	tmp = ptr;

	while (ptr && *ptr) {
		if (*ptr == '\'') {
			ptr = io_parser_parse_string_single(ptr);
		} else if (*ptr == '"') {
			ptr = io_parser_parse_string_double(ptr);
		} else if (*ptr == '[') {
			int n_equals = 0;
			while (ptr[n_equals + 1] == '=') n_equals++;
			if (ptr[n_equals + 1] == '[') {
				ptr = io_parser_parse_string_multiline(ptr,
					n_equals);
			}
		} else if (!strncmp(ptr, end_tag, end_tag_len)) {
			ptr += end_tag_len - 1;
			break;
		}
		if (ptr && *ptr) ptr++;
	}

	if (ptr) {
		if (*ptr == '\0') {
			value = sdsnewlen(tmp, ptr - tmp);
		} else {
			value = sdsnewlen(tmp, ptr - tmp - end_tag_len + 1);
		}
	} else {
		value = sdsnew(tmp);
	}
	gds_dlist_push(tokens, io_token_new(type, value));

	return ptr;
}

static const char * io_parser_parse_whitespace(const char *ptr,
	io_parser_context_t *context)
{
	gds_dlist_t *tokens = context->tokens;
	const char *tmp = ptr;
	sds value;

	while (*ptr == ' ' || *ptr == '\t') ptr++;

	value = sdsnewlen(tmp, ptr - tmp);
	gds_dlist_push(tokens, io_token_new(IO_TOKEN_TYPE_WHITESPACE, value));

	return ptr - 1;
}

static const char * io_parser_parse_text(const char *ptr,
	io_parser_context_t *context)
{
	gds_dlist_t *tokens = context->tokens;
	const char *start_tag = context->start_tag;
	size_t start_tag_len = context->start_tag_len;
	const char *tmp;
	sds value;

	while (ptr && *ptr) {
		if (*ptr == '\n') {
			gds_dlist_push(tokens, io_token_new(IO_TOKEN_TYPE_NEWLINE, NULL));
		} else if (*ptr == ' ' || *ptr == '\t') {
			ptr = io_parser_parse_whitespace(ptr, context);
		} else if (!strncmp(ptr, start_tag, start_tag_len)) {
			ptr = io_parser_parse_lua(ptr, context);
		} else {
			tmp = ptr;
			while (*ptr && *ptr != '\n' && *ptr != ' ' && *ptr != '\t' && strncmp(ptr, start_tag, start_tag_len)) {
				ptr++;
			}
			value = sdsnewlen(tmp, ptr - tmp);
			gds_dlist_push(tokens, io_token_new(IO_TOKEN_TYPE_TEXT, value));
			ptr--;
		}
		if (ptr && *ptr) ptr++;
	}

	return ptr;
}

sds io_parser_parse(const char *template, const char *start_tag, const char *end_tag)
{
	const char *ptr = template;
	io_token_t *token;
	sds buf;

	io_parser_context_t context;
	context.tokens = gds_dlist_new(io_token_free);
	context.start_tag = start_tag;
	context.end_tag = end_tag;
	context.start_tag_len = strlen(start_tag);
	context.end_tag_len = strlen(end_tag);

	io_parser_parse_text(ptr, &context);

	buf = sdsempty();
	gds_iterator_t *it = gds_dlist_iterator_new(context.tokens);
	while (!gds_iterator_step(it)) {
		token = gds_iterator_get(it);
		switch (token->type) {
			case IO_TOKEN_TYPE_PLAIN:
				buf = sdscat(buf, token->value);
				break;
			case IO_TOKEN_TYPE_TEXT:
				buf = sdscat(buf, "Io.output(");
				buf = sdscatrepr(buf, token->value, sdslen(token->value));
				buf = sdscat(buf, ");");
				break;
			case IO_TOKEN_TYPE_WHITESPACE:
				buf = sdscat(buf, "Io.output(\"");
				buf = sdscat(buf, token->value);
				buf = sdscat(buf, "\");");
				break;
			case IO_TOKEN_TYPE_NEWLINE:
				buf = sdscat(buf, "Io.output(\"\\n\");\n");
				break;
			case IO_TOKEN_TYPE_LUA_EXPR:
				buf = sdscat(buf, "Io.output(");
				buf = sdscat(buf, token->value);
				buf = sdscat(buf, ");");
				break;
		}
	}
	gds_iterator_free(it);
	gds_dlist_free(context.tokens);

	return buf;
}

sds io_parser_parse_filep(FILE *filep, const char *start_tag, const char *end_tag)
{
	char buf[1024];
	sds out;
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

	out = io_parser_parse(tpl, start_tag, end_tag);
	sdsfree(tpl);

	return out;
}

char * io_parser_parse_file(const char *filename, const char *start_tag,
	const char *end_tag)
{
	FILE *fp;
	sds out = NULL;

	fp = fopen(filename, "r");
	if (fp != NULL) {
		out = io_parser_parse_filep(fp, start_tag, end_tag);
		fclose(fp);
	}

	return out;
}
