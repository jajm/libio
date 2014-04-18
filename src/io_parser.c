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
#include "io_config.h"

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
	IO_TOKEN_TYPE_CODE,
	IO_TOKEN_TYPE_EXPR,
	IO_TOKEN_TYPE_COMMENT
} io_token_type_t;

typedef struct {
	io_token_type_t type;
	sds value;
	gds_inline_dlist_node_t dlist;
} io_token_t;

typedef struct {
	io_token_t *tokens_head;
	io_token_t *tokens_tail;
	io_config_t *config;
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

static const char * io_parser_parse_lua(const char *ptr, sds end_tag,
	io_token_type_t type, io_parser_context_t *context)
{
	size_t end_tag_len = sdslen(end_tag);
	const char *tmp;
	sds value;

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

static const char * io_parser_parse_code(const char *ptr,
	io_parser_context_t *context)
{
	ptr += sdslen(context->config->code_start_tag);
	return io_parser_parse_lua(ptr, context->config->code_end_tag,
		IO_TOKEN_TYPE_CODE, context);
}

static const char * io_parser_parse_expr(const char *ptr,
	io_parser_context_t *context)
{
	ptr += sdslen(context->config->expr_start_tag);
	return io_parser_parse_lua(ptr, context->config->expr_end_tag,
		IO_TOKEN_TYPE_EXPR, context);
}

static const char * io_parser_parse_comment(const char *ptr,
	io_parser_context_t *context)
{
	ptr += sdslen(context->config->comm_start_tag);
	return io_parser_parse_lua(ptr, context->config->comm_end_tag,
		IO_TOKEN_TYPE_COMMENT, context);
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
	sds code_start_tag = context->config->code_start_tag;
	size_t code_start_tag_len = sdslen(code_start_tag);
	sds expr_start_tag = context->config->expr_start_tag;
	size_t expr_start_tag_len = sdslen(expr_start_tag);
	sds comm_start_tag = context->config->comm_start_tag;
	size_t comm_start_tag_len = sdslen(comm_start_tag);
	const char *tmp;
	sds value;

	tmp = ptr;
	while (*ptr && *ptr != '\n' && *ptr != ' ' && *ptr != '\t'
	&& strncmp(ptr, code_start_tag, code_start_tag_len)
	&& strncmp(ptr, expr_start_tag, expr_start_tag_len)
	&& strncmp(ptr, comm_start_tag, comm_start_tag_len))
	{
		ptr++;
	}

	value = sdsnewlen(tmp, ptr - tmp);
	io_token_list_push(context, io_token_new(IO_TOKEN_TYPE_TEXT, value));

	return ptr - 1;
}

static const char * io_parser_parse_main(const char *ptr,
	io_parser_context_t *context)
{
	sds code_start_tag = context->config->code_start_tag;
	size_t code_start_tag_len = sdslen(code_start_tag);
	sds expr_start_tag = context->config->expr_start_tag;
	size_t expr_start_tag_len = sdslen(expr_start_tag);
	sds comm_start_tag = context->config->comm_start_tag;
	size_t comm_start_tag_len = sdslen(comm_start_tag);

	while (ptr && *ptr) {
		if (*ptr == '\n') {
			io_token_t *token = io_token_new(IO_TOKEN_TYPE_NEWLINE,
				sdsnew("\n"));
			io_token_list_push(context, token);
		} else if (*ptr == ' ' || *ptr == '\t') {
			ptr = io_parser_parse_whitespace(ptr, context);
		} else if (!strncmp(ptr, comm_start_tag, comm_start_tag_len)) {
			ptr = io_parser_parse_comment(ptr, context);
		} else if (!strncmp(ptr, expr_start_tag, expr_start_tag_len)) {
			ptr = io_parser_parse_expr(ptr, context);
		} else if (!strncmp(ptr, code_start_tag, code_start_tag_len)) {
			ptr = io_parser_parse_code(ptr, context);
		} else {
			ptr = io_parser_parse_text(ptr, context);
		}
		if (ptr && *ptr) ptr++;
	}

	return ptr;
}

static void io_parser_lua_token_set_chomps(io_token_t *token,
	io_chomp_type_t *pre, io_chomp_type_t *post)
{
	const char *ptr;
	int pre_flag = 1, post_flag = 1;

	ptr = token->value;
	switch (*ptr) {
		case '+': *pre = IO_CHOMP_NONE; break;
		case '-': *pre = IO_CHOMP_ONE; break;
		case ':': *pre = IO_CHOMP_COLLAPSE; break;
		case '~': *pre = IO_CHOMP_GREEDY; break;
		default: pre_flag = 0;
	}

	ptr = token->value + sdslen(token->value) - 1;
	switch (*ptr) {
		case '+': *post = IO_CHOMP_NONE; break;
		case '-': *post = IO_CHOMP_ONE; break;
		case ':': *post = IO_CHOMP_COLLAPSE; break;
		case '~': *post = IO_CHOMP_GREEDY; break;
		default: post_flag = 0;
	}

	sdsrange(token->value, pre_flag, -(post_flag + 1));
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
		if (token->type == IO_TOKEN_TYPE_CODE
		|| token->type == IO_TOKEN_TYPE_EXPR
		|| token->type == IO_TOKEN_TYPE_COMMENT)
		{
			io_parser_process_lua_token(context, token);
		}
		it = it->next;
	}
}

sds io_parser_parse(const char *template, io_config_t *config)
{
	const char *ptr = template;
	io_token_t *token;
	sds buf;
	gds_inline_dlist_node_t *it;

	io_parser_context_t context;
	context.tokens_head = NULL;
	context.tokens_tail = NULL;
	context.config = config;

	io_parser_parse_main(ptr, &context);

	io_parser_process_lua_tokens(&context);

	buf = sdsempty();
	it = &(context.tokens_head->dlist);
	while (it) {
		token = container_of(it, io_token_t, dlist);
		switch (token->type) {
			case IO_TOKEN_TYPE_PLAIN:
			case IO_TOKEN_TYPE_CODE:
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
			case IO_TOKEN_TYPE_EXPR:
				buf = sdscat(buf, "Io.output(");
				buf = sdscat(buf, token->value);
				buf = sdscat(buf, ");");
				break;
			case IO_TOKEN_TYPE_COMMENT:
				/* Do nothing */
				break;
		}
		it = it->next;
	}

	io_token_list_free(&context);

	return buf;
}

sds io_parser_parse_filep(FILE *filep, io_config_t *config)
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

	out = io_parser_parse(tpl, config);
	sdsfree(tpl);

	return out;
}

sds io_parser_parse_file(const char *filename, io_config_t *config)
{
	FILE *fp;
	sds out = NULL;

	fp = fopen(filename, "r");
	if (fp != NULL) {
		out = io_parser_parse_filep(fp, config);
		fclose(fp);
	}

	return out;
}
