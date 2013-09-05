DIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(DIR)/package.mk

CC := gcc
MAKE := make
LIBTOOL := libtool
INSTALL := install

PREFIX := /usr/local
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include/$(PACKAGE_NAME)
PKGCONFIGDIR := $(LIBDIR)/pkgconfig
