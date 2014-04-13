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

#define to_s(s) #s

static void test_simple_text(void)
{
	const char *tpl = "foo bar baz";
	const char *exp = to_s( Io.output("foo");Io.output(" "); )
		to_s( Io.output("bar");Io.output(" "); )
		to_s( Io.output("baz"); );

	test_parser_parse(tpl, exp, __func__);
}

static void test_simple_expr(void)
{
	const char *tpl = to_s( #{= "bar baz" }# );
	const char *exp = to_s( Io.output( "bar baz" ); );

	test_parser_parse(tpl, exp, __func__);
}

static void test_simple_lua(void)
{
	const char *tpl = to_s( #{ "bar baz" }# );
	const char *exp = " \"bar baz\" ";

	test_parser_parse(tpl, exp, __func__);
}

static void test_simple_text_with_quotes(void)
{
	const char *tpl = to_s( 'foo' "bar" [[baz]] );
	const char *exp = to_s( Io.output("'foo'");Io.output(" ");)
		to_s( Io.output("\"bar\"");Io.output(" ");)
		to_s( Io.output("[[baz]]"); );

	test_parser_parse(tpl, exp, __func__);
}

static void test_newlines(void)
{
	const char *tpl = "foobar\n"
		"\n"
		"baz\n"
		"\n"
		"#{ luacode\n"
		"\n"
		"luacode2;}#foo#{= luaexpr\n"
		"luaexpr2}#";
	const char *exp = to_s( Io.output("foobar");Io.output("\n"); ) "\n"
		to_s( Io.output("\n"); ) "\n"
		to_s( Io.output("baz");Io.output("\n"); ) "\n"
		to_s( Io.output("\n"); ) "\n"
		" luacode\n"
		"\n"
		to_s( luacode2;Io.output("foo"); ) "Io.output( luaexpr\n"
		"luaexpr2);";

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
	plan(6);

	test_simple_text();
	test_simple_expr();
	test_simple_lua();
	test_simple_text_with_quotes();
	test_newlines();
	test_unterminated_string();

	return 0;
}
