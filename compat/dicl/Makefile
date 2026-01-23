# Makefile for Mogrix libdicl-compat library
#
# This builds a library compatible with libdicl-0.1 that can be used
# with rpm and other packages that expect libdicl.

CC ?= cc
AR ?= ar
RANLIB ?= ranlib

CFLAGS ?= -O2 -fPIC
CPPFLAGS ?= -D_SGI_SOURCE

# Library versions
ABI_VERSION = 0.1
SONAME = libdicl-$(ABI_VERSION).so

# Install directories
PREFIX ?= /usr/sgug
LIBDIR ?= $(PREFIX)/lib32
INCLUDEDIR ?= $(PREFIX)/include/libdicl-$(ABI_VERSION)

# Sources
SOURCES = dicl-compat.c
OBJECTS = $(SOURCES:.c=.o)

# Targets
STATIC_LIB = libdicl-$(ABI_VERSION).a
SHARED_LIB = libdicl-$(ABI_VERSION).so

.PHONY: all clean install

all: $(STATIC_LIB) $(SHARED_LIB)

$(STATIC_LIB): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)
	$(RANLIB) $@

$(SHARED_LIB): $(OBJECTS)
	$(CC) -shared -o $@ $(OBJECTS) -lpthread

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(STATIC_LIB) $(SHARED_LIB)

install: all
	mkdir -p $(DESTDIR)$(LIBDIR)
	mkdir -p $(DESTDIR)$(INCLUDEDIR)/libdicl
	cp $(STATIC_LIB) $(DESTDIR)$(LIBDIR)/
	cp $(SHARED_LIB) $(DESTDIR)$(LIBDIR)/
	cp stdlib.h $(DESTDIR)$(INCLUDEDIR)/
	cp libdicl.h $(DESTDIR)$(INCLUDEDIR)/libdicl/
