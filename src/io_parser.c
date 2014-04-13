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

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sds.h>
#include <libgends/inline/dlist.h>

typedef enum {
	IO_CHOMP_NONE = 0,
	IO_CHOMP_ONE,
	IO_CHOMP_COLLAPSE,
	IO_CHOMP_GREEDY
} io_chomp_type_t;

typedef enum {
	IO_TOKEN_TYPE_PLAIN,
	IO_TOKEN_TYPE_TEXT,
	IO_TOKEN_TYPE_WHITESPACE,
	IO_TOKEN_TYPE_NEWLINE,
	IO_TOKEN_TYPE_LUA,
	IO_TOKEN_TYPE_LUA_EXPR
} io_token_type_t;

typedef struct {
	io_token_type_t type;
	sds value;
	gds_inline_dlist_node_t dlist;
} io_token_t;

typedef struct {
	io_token_t *tokens_head;
	io_token_t *tokens_tail;
	const char *start_tag;
	const char *end_tag;
	size_t start_tag_len;
	size_t end_tag_len;
} io_parser_context_t;

static io_token_t * io_token_new(io_token_type_t type, sds value)
{
	io_token_t *token;

	token = malloc(sizeof(io_token_t));
	token->type = type;
	token->value = value;
	gds_inline_dlist_node_init(&(token->dlist));

	return token;
}

static void io_token_free(io_token_t *token)
{
	if (token) {
		sdsfree(token->value);
		free(token);
	}
}

#define container_of(ptr, type, member) ({ \
	const __typeof__( ((type *)0)->member ) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member) ); \
})

static void io_token_list_push(io_parser_context_t *context, io_token_t *token)
{
	gds_inline_dlist_node_t *itail;

	if (context->tokens_head) {
		itail = &(context->tokens_tail->dlist);
		gds_inline_dlist_node_append(itail, &(token->dlist), &itail);
		context->tokens_tail = container_of(itail, io_token_t, dlist);
	} else {
		context->tokens_head = context->tokens_tail = token;
	}
}

static void io_token_list_free_callback(gds_inline_dlist_node_t *node)
{
	io_token_t *token;

	token = container_of(node, io_token_t, dlist);
	io_token_free(token);
}

static void io_token_list_free(io_parser_context_t *context)
{
	gds_inline_dlist_node_t *head;

	head = &(context->tokens_head->dlist);
	gds_inline_dlist_remove(head, 0, gds_inline_dlist_size(head),
		io_token_list_free_callback, NULL, NULL);
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
	const char *end_tag = context->end_tag;
	size_t end_tag_len = context->end_tag_len;
	const char *tmp;
	io_token_type_t type = IO_TOKEN_TYPE_LUA;
	sds value;

	ptr += context->start_tag_len;
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

	io_token_list_push(context, io_token_new(type, value));

	return ptr;
}

static const char * io_parser_parse_whitespace(const char *ptr,
	io_parser_context_t *context)
{
	const char *tmp = ptr;
	sds value;

	while (*ptr == ' ' || *ptr == '\t') ptr++;

	value = sdsnewlen(tmp, ptr - tmp);
	io_token_list_push(context,
		io_token_new(IO_TOKEN_TYPE_WHITESPACE, value));

	return ptr - 1;
}

static const char * io_parser_parse_text(const char *ptr,
	io_parser_context_t *context)
{
	const char *start_tag = context->start_tag;
	size_t start_tag_len = context->start_tag_len;
	const char *tmp;
	sds value;

	while (ptr && *ptr) {
		if (*ptr == '\n') {
			io_token_list_push(context,
				io_token_new(IO_TOKEN_TYPE_NEWLINE, sdsnew("\n")));
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
			io_token_list_push(context,
				io_token_new(IO_TOKEN_TYPE_TEXT, value));
			ptr--;
		}
		if (ptr && *ptr) ptr++;
	}

	return ptr;
}

static void io_parser_lua_token_set_chomps(io_token_t *token,
	io_chomp_type_t *pre, io_chomp_type_t *post)
{
	const char *ptr;
	int done = 0;
	int pre_flags = -1, post_flags = -1;

	ptr = token->value;
	while (*ptr && !done) {
		switch (*ptr) {
			case '+': *pre = IO_CHOMP_NONE; break;
			case '-': *pre = IO_CHOMP_ONE; break;
			case ':': *pre = IO_CHOMP_COLLAPSE; break;
			case '~': *pre = IO_CHOMP_GREEDY; break;
			default:  done = 1;
		}
		pre_flags++;
		ptr++;
	}

	done = 0;
	ptr = token->value + sdslen(token->value) - 1;
	while (*ptr && !done) {
		switch (*ptr) {
			case '+': *post = IO_CHOMP_NONE; break;
			case '-': *post = IO_CHOMP_ONE; break;
			case ':': *post = IO_CHOMP_COLLAPSE; break;
			case '~': *post = IO_CHOMP_GREEDY; break;
			default:  done = 1;
		}
		post_flags++;
		ptr--;
	}

	sdsrange(token->value, pre_flags, -(post_flags + 1));
}

static void io_parser_lua_token_pre_chomp(io_token_t *token,
	io_chomp_type_t chomp)
{
	io_token_type_t oldtype;
	gds_inline_dlist_node_t *it;

	if (!chomp) return;

	it = token->dlist.prev;
	while (it) {
		token = container_of(it, io_token_t, dlist);

		if (token->type != IO_TOKEN_TYPE_WHITESPACE
		&& token->type != IO_TOKEN_TYPE_NEWLINE) {
			break;
		}

		oldtype = token->type;
		token->type = IO_TOKEN_TYPE_PLAIN;

		if (chomp == IO_CHOMP_ONE && oldtype == IO_TOKEN_TYPE_NEWLINE) {
			break;
		}

		it = it->prev;
	}
}

static void io_parser_lua_token_post_chomp(io_token_t *token,
	io_chomp_type_t chomp)
{
	io_token_type_t oldtype;
	gds_inline_dlist_node_t *it;

	if (!chomp) return;

	it = token->dlist.next;
	while (it) {
		token = container_of(it, io_token_t, dlist);

		if (token->type != IO_TOKEN_TYPE_WHITESPACE
		&& token->type != IO_TOKEN_TYPE_NEWLINE) {
			break;
		}

		oldtype = token->type;
		token->type = IO_TOKEN_TYPE_PLAIN;

		if (chomp == IO_CHOMP_ONE && oldtype == IO_TOKEN_TYPE_NEWLINE) {
			break;
		}

		it = it->next;
	}
}

static void io_parser_process_lua_token(io_parser_context_t *context,
	io_token_t *token)
{
	io_chomp_type_t pre_chomp = 0, post_chomp = 0;
	io_token_t *newtoken;

	io_parser_lua_token_set_chomps(token, &pre_chomp, &post_chomp);
	if (token->value[0] == '=') {
		token->type = IO_TOKEN_TYPE_LUA_EXPR;
		sdsrange(token->value, 1, -1);
	}
	io_parser_lua_token_pre_chomp(token, pre_chomp);
	io_parser_lua_token_post_chomp(token, post_chomp);

	if (pre_chomp == IO_CHOMP_COLLAPSE) {
		newtoken = io_token_new(IO_TOKEN_TYPE_WHITESPACE, sdsnew(" "));
		gds_inline_dlist_node_prepend(&(token->dlist), &(newtoken->dlist), NULL);
		if (newtoken->dlist.prev == NULL) {
			context->tokens_head = newtoken;
		}
	}
	if (post_chomp == IO_CHOMP_COLLAPSE) {
		newtoken = io_token_new(IO_TOKEN_TYPE_WHITESPACE, sdsnew(" "));
		gds_inline_dlist_node_append(&(token->dlist), &(newtoken->dlist), NULL);
		if (newtoken->dlist.next == NULL) {
			context->tokens_tail = newtoken;
		}
	}
}

static void io_parser_process_lua_tokens(io_parser_context_t *context)
{
	io_token_t *token;
	gds_inline_dlist_node_t *it;

	it = &(context->tokens_head->dlist);
	while (it) {
		token = container_of(it, io_token_t, dlist);
		if (token->type == IO_TOKEN_TYPE_LUA) {
			io_parser_process_lua_token(context, token);
		}
		it = it->next;
	}
}

sds io_parser_parse(const char *template, const char *start_tag, const char *end_tag)
{
	const char *ptr = template;
	io_token_t *token;
	sds buf;
	gds_inline_dlist_node_t *it;

	io_parser_context_t context;
	context.tokens_head = NULL;
	context.tokens_tail = NULL;
	context.start_tag = start_tag;
	context.end_tag = end_tag;
	context.start_tag_len = strlen(start_tag);
	context.end_tag_len = strlen(end_tag);

	io_parser_parse_text(ptr, &context);

	io_parser_process_lua_tokens(&context);

	buf = sdsempty();
	it = &(context.tokens_head->dlist);
	while (it) {
		token = container_of(it, io_token_t, dlist);
		switch (token->type) {
			case IO_TOKEN_TYPE_PLAIN:
			case IO_TOKEN_TYPE_LUA:
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
		it = it->next;
	}

	io_token_list_free(&context);

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
