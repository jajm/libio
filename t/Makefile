include ../package.mk
include ../config.mk

CFLAGS := -Wall -Wextra -Werror -g -std=c99 $(CFLAGS)
CPPFLAGS := -I../include $(shell pkg-config --cflags libobject) $(CPPFLAGS)
LDFLAGS := $(shell pkg-config --libs libobject) $(LDFLAGS)

PROGRAMS := $(patsubst %.c,%.t,$(wildcard *.c))

all: $(PROGRAMS)

$(PROGRAMS): %.t : %.o ../src/$(LIBRARY_NAME)
	$(LIBTOOL) --mode=link $(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o .libs $(PROGRAMS)
