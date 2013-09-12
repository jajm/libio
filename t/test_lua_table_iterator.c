#include <libobject/integer.h>
#include <libobject/real.h>
#include <libobject/string.h>
#include "lua_table_iterator.h"
#include "tap.h"

void io_object_dump(object_t *o)
{
	if (object_is_integer(o)) {
		printf("[integer] %d", integer_get(o));
	} else if (object_is_real(o)) {
		printf("[number] %lf", real_get(o));
	} else if (object_is_string(o)) {
		printf("[string] \"%s\"", string_to_c_str(o));
	} else {
		printf("[unknown]");
	}
}

int main()
{
	object_t *k, *v, *t;
	io_lua_table_iterator_t *it;
	int i;

	plan(2);

	t = io_lua_table();

	k = string("Blob");
	v = string("Blob value");
	io_lua_table_set(t, k, v);

	k = integer(42);
	v = real(42.24);
	io_lua_table_set(t, k, v);

	it = io_lua_table_iterator(t);
	ok(it != NULL, "it is not NULL");
	i = 0;
	while (!io_lua_table_iterator_step(it)) {
		//k = io_table_iterator_getkey(it);
		//v = io_table_iterator_getvalue(it);

		//printf("key = ");
		//io_value_dump(k);
		//printf("\nvalue = ");
		//io_value_dump(v);
		//printf("\n");

		i++;
	}

	ok(i == 2, "table contains 2 elements");

}
