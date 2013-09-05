include ../package.mk
include ../config.mk

CFLAGS := -Wall -Wextra -Werror -g -std=c99 $(CFLAGS)
CPPFLAGS := -I../include $(shell pkg-config --cflags lua5.2) $(CPPFLAGS)

PROGRAMS := $(patsubst %.c,%.t,$(wildcard *.c))

all: $(PROGRAMS)

$(PROGRAMS): %.t : %.o ../src/$(LIBRARY_NAME)
	$(LIBTOOL) --mode=link $(CC) -o $@ $^

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o .libs $(PROGRAMS)
