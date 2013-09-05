#include "value.h"
#include "table_iterator.h"
#include "tap.h"

void io_value_dump(io_value_t *v)
{
	switch (io_value_type(v)) {
		case IO_VALUE_INTEGER:
			printf("[integer] %ld", io_value_integer_get(v));
			break;
		case IO_VALUE_NUMBER:
			printf("[number] %lf", io_value_number_get(v));
			break;
		case IO_VALUE_STRING:
			printf("[string] \"%s\"", io_value_string_get(v));
			break;
		default:
			printf("[unknown]");
	}
}

int main()
{
	io_value_t *k, *v, *t;
	io_table_iterator_t *it;
	int i;

	plan(2);

	t = io_value_table();

	k = io_value_string("Blob");
	v = io_value_string("Blob value");
	io_value_table_set(t, k, v);

	k = io_value_integer(42);
	v = io_value_number(42.24);
	io_value_table_set(t, k, v);

	it = io_table_iterator(t);
	ok(it != NULL, "it is not NULL");
	i = 0;
	while (!io_table_iterator_step(it)) {
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
