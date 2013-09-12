#include <libobject/object.h>
#include "cfunction.h"

static const char IO_CFUNCTION_TYPE[] = "Io:cfunction";

io_cfunction_t * io_cfunction(lua_CFunction f)
{
	io_cfunction_t *cfunction;

	cfunction = object_new(IO_CFUNCTION_TYPE, f);

	return cfunction;
}

int io_cfunction_set(io_cfunction_t *cf, lua_CFunction f)
{
	if (object_isset(cf)) {
		object_set(cf, f);
	}

	return 0;
}

lua_CFunction io_cfunction_get(const io_cfunction_t *cf)
{
	return object_value(cf);
}

int object_is_cfunction(const object_t *object)
{
	if (object_isa(object, IO_CFUNCTION_TYPE))
		return 1;

	return 0;
}
