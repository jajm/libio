#include <stdlib.h>
#include <stdio.h>
#include <libobject/string.h>
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

	plan(2);

	io_template_t *T = io_template_new(tpl);
	ok(T != NULL, "T is not NULL");
	if (T) {
		io_template_param(T, "name", string("world!"));
		out = io_template_render(T);
		ok(strcmp(out,
			"Hello, world! \n"
			"Test inclusion WORLD!\n"
			"\n"
			"Hello again, WORLD?\n") == 0, "output is ok");
		io_template_free(T);
	}

}
