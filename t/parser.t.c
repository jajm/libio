#include <stdio.h>
#include <stdlib.h>
#include <sds.h>
#include <libtap13/tap.h>
#include "io_parser.h"
#include "io_config.h"

static void test_parser_parse(const char *tpl, const char *exp, const char *msg)
{
	sds code;
	io_config_t *config = io_config_new_default();
	
	code = io_parser_parse(tpl, config);
	if (code) {
		str_eq(code, exp, msg);
	}

	io_config_free(config);
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
	const char *tpl = to_s( {{ "bar baz" }} );
	const char *exp = to_s( Io.output( "bar baz" ); );

	test_parser_parse(tpl, exp, __func__);
}

static void test_simple_code(void)
{
	const char *tpl = to_s( {% "bar baz" %} );
	const char *exp = " \"bar baz\" ";

	test_parser_parse(tpl, exp, __func__);
}

static void test_simple_comment(void)
{
	const char *tpl = "foo{# comment #}bar";
	const char *exp = "Io.output(\"foo\");Io.output(\"bar\");";

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
		"{% luacode\n"
		"\n"
		"luacode2;%}foo{{ luaexpr\n"
		"luaexpr2}}";
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
	const char *tpl = "{{ 'foo";
	const char *exp = "Io.output( 'foo);";

	test_parser_parse(tpl, exp, __func__);
}

static void test_pre_chomp_one(void)
{
	const char *tpl = "foo\n"
		"  \n"
		"\t  {{- 'foo' }}";
	const char *exp = to_s( Io.output("foo");Io.output("\n"); ) "\n"
		to_s( Io.output("  "); ) "\n"
		"\t  Io.output( 'foo' );";

	test_parser_parse(tpl, exp, __func__);
}

static void test_post_chomp_one(void)
{
	const char *tpl = "{{ 'foo' -}}   \n"
		"  bar";
	const char *exp = "Io.output( 'foo' );   \n"
		to_s( Io.output("  ");Io.output("bar"); );

	test_parser_parse(tpl, exp, __func__);
}

static void test_pre_chomp_collapse(void)
{
	const char *tpl = "foo\n"
		"  \n"
		"\t  {{: 'foo' }}";
	const char *exp = to_s( Io.output("foo"); ) "\n"
		"  \n"
		"\t  Io.output(\" \");Io.output( 'foo' );";

	test_parser_parse(tpl, exp, __func__);
}

static void test_post_chomp_collapse(void)
{
	const char *tpl = "{{ 'foo' :}}  \n"
		"  \n"
		"\t  bar";
	const char *exp = "Io.output( 'foo' );Io.output(\" \");  \n"
		"  \n"
		"\t  Io.output(\"bar\");";

	test_parser_parse(tpl, exp, __func__);
}

static void test_pre_chomp_greedy(void)
{
	const char *tpl = "foo\n"
		"  \n"
		"\t  {{~ 'foo' }}";
	const char *exp = to_s( Io.output("foo"); ) "\n"
		"  \n"
		"\t  Io.output( 'foo' );";

	test_parser_parse(tpl, exp, __func__);
}

static void test_post_chomp_greedy(void)
{
	const char *tpl = "{{ 'foo' ~}}  \n"
		"  \n"
		"\t  bar";
	const char *exp = "Io.output( 'foo' );  \n"
		"  \n"
		"\t  Io.output(\"bar\");";

	test_parser_parse(tpl, exp, __func__);
}

int main()
{
	plan(13);

	test_simple_text();
	test_simple_expr();
	test_simple_code();
	test_simple_comment();
	test_simple_text_with_quotes();
	test_newlines();
	test_unterminated_string();

	test_pre_chomp_one();
	test_post_chomp_one();
	test_pre_chomp_collapse();
	test_post_chomp_collapse();
	test_pre_chomp_greedy();
	test_post_chomp_greedy();

	return 0;
}
