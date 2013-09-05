#ifndef object_t_tap_h_included
#define object_t_tap_h_included

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static unsigned int current = 0;

#define plan(n) \
	printf("1..%d\n", n)

#define ok(test, msg, ...) \
	do { \
		const char *_msg = msg; \
		current++; \
		if (test) printf("ok %d ", current); \
		else printf("not ok %d ", current); \
		if (_msg != NULL) printf(_msg, ##__VA_ARGS__); \
		printf("\n"); \
	} while(0)

#define str_eq(got, expected) \
	do { \
		const char *_got = got, *_expected = expected; \
		ok(strcmp(_got, _expected) == 0, \
			"string is \"%s\" (got: \"%s\")", \
			_expected, _got); \
	} while(0)

#define diag(msg, ...) \
	do { \
		printf("# "); \
		printf(msg, ##__VA_ARGS__); \
		printf("\n"); \
	} while(0)

#endif /* ! object_t_tap_h_included */

