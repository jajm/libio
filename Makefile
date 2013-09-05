include config.mk

PKGCONFIG_FILES := $(wildcard *.pc)

.PHONY: src test clean

all: lib test

lib:
	$(MAKE) -C src

test:
	$(MAKE) -C t

install:
	$(MAKE) -C src install
	$(MAKE) -C include install
	@ if [ -n "$(PKGCONFIG_FILES)" ]; then \
		CMD="$(INSTALL) --mode=0644 $(PKGCONFIG_FILES) $(PKGCONFIGDIR)"; \
		echo "$${CMD}"; \
		$${CMD}; \
	fi

clean:
	$(MAKE) -C src clean
	$(MAKE) -C t clean
