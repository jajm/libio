#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgends/hash_map.h>
#include <embody/embody.h>
#include <sds.h>
#include "lua_table.h"
#include "template.h"
#include "io_config.h"
#include "tap.h"

static void test_include(int argc, char **argv)
{
	sds progname;
	const char *progdir;
	io_config_t *config;
	const char *out;
	static const char *tpl =
		"Hello, #{= name }# #{\n"
		"	name2 = string.gsub(name, \"!\", \"?\")\n"
		"}#\n"
		"#{ Io.include('test.inc') }#\n"
		"Hello again, #{= name2:upper() }#\n";

	if (argc) {
		sds path;
		progname = sdsnew(argv[0]);
		progdir = dirname(progname);
		config = io_config_new();
		path = sdsnew(progdir);
		/* progdir looks like "/path/to/libio/t/.libs" */
		path = sdscat(path, "/../files");
		gds_slist_unshift(config->directories, path);
		sdsfree(progname);
	}

	io_template_t *T = io_template_new(config);
	io_template_set_template_string(T, tpl);
	if (T) {
		io_template_param(T, "name", emb_new("sds", sdsnew("world!")));
		out = io_template_render(T);
		ok(strcmp(out,
			"Hello, world! \n"
			"Test inclusion WORLD!\n"
			"\n"
			"Hello again, WORLD?\n") == 0, "output is ok");
		io_template_free(T);
	}
	io_config_free(config);
}

static void test_types(void)
{
	io_template_t *T;
	const char *out;
	const char *tpl =
		"Boolean: #{= boolean_value }#\n"
		"Integer: #{= integer_value }#\n"
		"Table element: #{= mytable.element }#\n"
		"List: #{ for i,v in ipairs(mylist) do }##{= v .. ',' }##{ end }#\n"
	;

	T = io_template_new(NULL);
	io_template_set_template_string(T, tpl);
	io_template_param(T, "boolean_value", emb_new_bool(true));
	io_template_param(T, "integer_value", emb_new_int8(127));
	gds_hash_map_t *table = io_lua_table_new();
	gds_hash_map_set(table, emb_new("sds", sdsnew("element")), emb_new_ushort(32769));
	io_template_param(T, "mytable", emb_new("gds_hash_map", table));
	gds_slist_t *list = gds_slist_new(emb_container_free);
	gds_slist_push(list, emb_new_int8(1), emb_new_int8(2), emb_new_int8(3));
	io_template_param(T, "mylist", emb_new("gds_slist", list));
	out = io_template_render(T);
	ok(!strcmp(out,
		"Boolean: 1\n"
		"Integer: 127\n"
		"Table element: 32769\n"
		"List: 1,2,3,\n"
	), "output of tpl is ok");
	io_template_free(T);
}

void test_end_tag_in_string(void)
{
	io_template_t *T;
	const char *out;
	const char *tpl = "'#{= \"}#\" }#'";
	const char *expected = "'}#'";
	const char *tpl2 = "\"#{= '}#' }#\"";
	const char *expected2 = "\"}#\"";
	const char *tpl3 = "#{ Io.output(\"\\\"}#\\\"\") }#";
	const char *expected3 = "\"}#\"";
	const char *tpl4 = "#{ Io.output('\\'}#\\'') }#";
	const char *expected4 = "'}#'";
	const char *tpl5 = "#{= [[ }# ]] }#";
	const char *expected5 = " }# ";
	const char *tpl6 = "#{= [====[ }# ]====] }#";
	const char *expected6 = " }# ";

	T = io_template_new(NULL);
	io_template_set_template_string(T, tpl);
	out = io_template_render(T);
	ok(!strcmp(out, expected), "end tag in double quoted string");

	io_template_set_template_string(T, tpl2);
	out = io_template_render(T);
	ok(!strcmp(out, expected2), "end tag in single quoted string");

	io_template_set_template_string(T, tpl3);
	out = io_template_render(T);
	ok(!strcmp(out, expected3), "end tag in double quoted string with escaped double quotes");

	io_template_set_template_string(T, tpl4);
	out = io_template_render(T);
	ok(!strcmp(out, expected4), "end tag in single quoted string with escaped single quotes");

	io_template_set_template_string(T, tpl5);
	out = io_template_render(T);
	ok(!strcmp(out, expected5), "end tag in multiline quoted string");

	io_template_set_template_string(T, tpl6);
	out = io_template_render(T);
	ok(!strcmp(out, expected6), "end tag in multiline quoted string (with equal signs)");

	io_template_free(T);
}

int main(int argc, char **argv)
{
	plan(8);

	test_include(argc, argv);
	test_types();
	test_end_tag_in_string();

	return 0;
}
