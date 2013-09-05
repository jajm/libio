#include "tap.h"
#include "value.h"

int main()
{
	plan(21);
	
	io_value_t *v, *k, *val, *u, *tmp;

	v = io_value_nil();
	ok(io_value_isa(v, IO_VALUE_NIL), "v is a nil");
	ok(v != NULL, "nil is not NULL");
	io_value_free(v);

	v = io_value_boolean(0);
	ok(io_value_isa(v, IO_VALUE_BOOLEAN), "v is a boolean");
	ok(!io_value_boolean_get(v), "io_value_boolean(0) is false");
	io_value_boolean_set(v, 1);
	ok(io_value_boolean_get(v), "io_value_boolean(1) is true");
	io_value_free(v);

	v = io_value_integer(0);
	ok(io_value_isa(v, IO_VALUE_INTEGER), "v is an integer");
	ok(io_value_integer_get(v) == 0, "io_value_integer(0) == 0");
	io_value_integer_set(v, -42);
	ok(io_value_integer_get(v) == -42, "io_value_integer(-42) == -42");
	io_value_free(v);

	v = io_value_number(1.23);
	ok(io_value_isa(v, IO_VALUE_NUMBER), "v is a number");
	ok(io_value_number_get(v) == 1.23, "io_value_number(1.23) == 1.23");
	io_value_number_set(v, 1);
	ok(io_value_number_get(v) == 1, "io_value_number(1) == 1");
	io_value_free(v);

	v = io_value_string("Hello, world!");
	ok(io_value_isa(v, IO_VALUE_STRING), "v is a string");
	ok(strcmp(io_value_string_get(v), "Hello, world!") == 0, "io_value_string(\"Hello, world!\") == \"Hello, world!\"");
	io_value_string_set(v, "Goodbye, world...");
	ok(strcmp(io_value_string_get(v), "Goodbye, world...") == 0, "io_value_string(\"Goodbye, world...\") == \"Goodbye, world...\"");
	io_value_free(v);

	int my_lua_function(lua_State *L) { if (L) {} return 0; }
	int my_lua_function2(lua_State *L) { if (L) {} return 0; }
	v = io_value_cfunction(my_lua_function);
	ok(io_value_isa(v, IO_VALUE_CFUNCTION), "v is a cfunction");
	ok(io_value_cfunction_get(v) == my_lua_function, "io_value_cfunction(my_lua_function) == my_lua_function");
	io_value_cfunction_set(v, my_lua_function2);
	ok(io_value_cfunction_get(v) == my_lua_function2, "io_value_cfunction(my_lua_function2) == my_lua_function2");
	io_value_free(v);

	v = io_value_table();
	ok(v != NULL, "v is not NULL");
	k = io_value_string("my_key");
	val = io_value_integer(42);
	io_value_table_set(v, k, val);
	tmp = io_value_table_get(v, k);
	ok(tmp == val, "v.my_key == 42");
	u = io_value_copy(v);
	ok(u != v, "io_value_copy(v) != v");
	tmp = io_value_table_get(u, k);
	ok(io_value_integer_get(tmp) == 42, "u.my_key == 42");
	io_value_free(v);
	io_value_free(u);
}
