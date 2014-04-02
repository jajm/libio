#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgends/hash_map.h>
#include <embody/embody.h>
#include <sds.h>
#include "lua_table.h"
#include "template.h"
#include "tap.h"

int main()
{
	const char *out;
	static const char *tpl =
		"Hello, #{= name }# #{\n"
		"	name2 = string.gsub(name, \"!\", \"?\")\n"
		"}#\n"
		"#{ Io.include('t/files/test.inc') }#\n"
		"Hello again, #{= name2:upper() }#\n";

	plan(3);

	io_template_t *T = io_template_new(tpl);
	ok(T != NULL, "T is not NULL");
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

	const char *tpl2 =
		"Boolean: #{= boolean_value }#\n"
		"Integer: #{= integer_value }#\n"
		"Table element: #{= mytable.element }#\n"
	;
	T = io_template_new(tpl2);
	io_template_param(T, "boolean_value", (void **)emb_new_bool(true));
	io_template_param(T, "integer_value", (void **)emb_new_int8(127));
	gds_hash_map_t *table = io_lua_table_new();
	gds_hash_map_set(table, emb_new("sds", sdsnew("element")), emb_new_short(32767));
	io_template_param(T, "mytable", emb_new("gds_hash_map", table));
	out = io_template_render(T);
	ok(!strcmp(out,
		"Boolean: 1\n"
		"Integer: 127\n"
		"Table element: 32767\n"), "output of tpl2 is ok");
	io_template_free(T);
}
