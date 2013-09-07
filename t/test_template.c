#include <stdlib.h>
#include <stdio.h>
#include "template.h"
#include "tap.h"

int main()
{
	char *out;
	static const char *tpl =
		"Hello, #{= name }# #{\n"
		"	local name2 = string.gsub(name, \"!\", \"?\")\n"
		"}#\n"
		"Hello again, #{= name2 }#\n";
	
	plan(2);

	io_template_t *T = io_template_new(tpl);
	ok(T != NULL, "T is not NULL");
	if (T) {
		io_template_param(T, "name", io_value_string("world!"));
		out = io_template_render(T);
		ok(strcmp(out,
			"Hello, world! \n"
			"Hello again, world?\n") == 0, "output is ok");
		free(out);
		io_template_free(T);
	}
}
