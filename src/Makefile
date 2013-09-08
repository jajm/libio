include ../package.mk
include ../config.mk

CFLAGS := -Wall -Wextra -Werror -g -std=c99 $(CFLAGS)
CPPFLAGS := -I../include -I/usr/local/include $(shell pkg-config --cflags lua5.2 libgends) $(CPPFLAGS)
LDFLAGS := $(shell pkg-config --libs lua5.2 libgends) -lobject $(LDFLAGS)
LIBTOOL_VERSION := $(LIBTOOL_CURRENT):$(LIBTOOL_REVISION):$(LIBTOOL_AGE)

C_SOURCES := $(wildcard *.c)
C_OBJS := $(C_SOURCES:.c=.lo)
OBJS := $(C_OBJS)

all: $(LIBRARY_NAME)

$(LIBRARY_NAME): $(OBJS)
	$(LIBTOOL) --mode=link $(CC) $(LDFLAGS) -rpath $(LIBDIR) -version-info $(LIBTOOL_VERSION) -release $(PACKAGE_RELEASE) -o $@ $^

.deps/%.d: %.c
	@set -e; rm -f $@; \
	mkdir -p $(dir $@); \
	$(CC) -MM -MG $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(patsubst %.c,.deps/%.d,$(C_SOURCES))

%.lo: %.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

install: $(LIBRARY_NAME)
	$(LIBTOOL) --mode=install $(INSTALL) $< $(LIBDIR)

clean:
	rm -rf *.o *.lo $(LIBRARY_NAME)
	rm -rf .deps .libs
