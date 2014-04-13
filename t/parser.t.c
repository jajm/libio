#include <stdio.h>
#include <stdlib.h>
#include <sds.h>
#include <libtap13/tap.h>
#include "io_parser.h"

static void test_simple_text(void)
{
	const char *tpl1 = "foo bar baz";
	const char *exp1 = "Io.output(\"foo\");Io.output(\" \");"
		"Io.output(\"bar\");Io.output(\" \");"
		"Io.output(\"baz\");";
	sds code;

	code = io_parser_parse(tpl1, "#{", "}#");
	if (code) {
		ok(!strcmp(code, exp1), "output is ok");
	}

	sdsfree(code);
}

static void test_simple_expr(void)
{
	const char *tpl1 = "#{= \"bar baz\" }#";
	const char *exp1 = "Io.output( \"bar baz\" );";
	sds code;

	code = io_parser_parse(tpl1, "#{", "}#");
	if (code) {
		ok(!strcmp(code, exp1), "output is ok");
	}

	sdsfree(code);
}

static void test_unterminated_string(void)
{
	const char *tpl1 = "#{= 'foo";
	const char *exp1 = "Io.output( 'foo);";
	sds code;

	code = io_parser_parse(tpl1, "#{", "}#");
	if (code) {
		ok(!strcmp(code, exp1), "output is ok");
	}

	sdsfree(code);
}

int main()
{
	plan(3);

	test_simple_text();
	test_simple_expr();
	test_unterminated_string();

	return 0;
}
