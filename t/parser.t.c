#include <stdio.h>
#include <stdlib.h>
#include <sds.h>
#include <libtap13/tap.h>
#include "io_parser.h"

static void test_parser_parse(const char *tpl, const char *exp, const char *msg)
{
	sds code;

	code = io_parser_parse(tpl, "#{", "}#");
	if (code) {
		str_eq(code, exp, msg);
	}

	sdsfree(code);
}

static void test_simple_text(void)
{
	const char *tpl = "foo bar baz";
	const char *exp = "Io.output(\"foo\");Io.output(\" \");"
		"Io.output(\"bar\");Io.output(\" \");"
		"Io.output(\"baz\");";

	test_parser_parse(tpl, exp, __func__);
}

static void test_simple_expr(void)
{
	const char *tpl = "#{= \"bar baz\" }#";
	const char *exp = "Io.output( \"bar baz\" );";

	test_parser_parse(tpl, exp, __func__);
}

static void test_unterminated_string(void)
{
	const char *tpl = "#{= 'foo";
	const char *exp = "Io.output( 'foo);";

	test_parser_parse(tpl, exp, __func__);
}

int main()
{
	plan(3);

	test_simple_text();
	test_simple_expr();
	test_unterminated_string();

	return 0;
}
