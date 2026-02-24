PKG_CONFIG ?= pkg-config
CC ?= gcc
PREFIX ?= /usr/sgug
DESTDIR ?=

CFLAGS_PKG = $(shell $(PKG_CONFIG) --cflags webkit2gtk-4.0)
LIBS_PKG   = $(shell $(PKG_CONFIG) --libs webkit2gtk-4.0)

# IRIX n32: sizeof(gpointer)=4 but sizeof(GType/gulong)=8.
# GLib 2.80 g_once_init_enter_pointer needs sizeof(*loc)==sizeof(gpointer).
# Force old gsize-based path by capping GLib API version.
CFLAGS += -Wall -O2 -DGLIB_VERSION_MIN_REQUIRED=G_ENCODE_VERSION\(2,78\) -DGLIB_VERSION_MAX_ALLOWED=G_ENCODE_VERSION\(2,78\) $(CFLAGS_PKG)
LDFLAGS += $(LIBS_PKG)

SRCS = main.c ir8-window.c ir8-tab.c ir8-searchbox.c ir8-downloads.c \
       ir8-settings.c ir8-cellrenderer.c ir8-bookmarks.c ir8-marshal.c
HDRS = ir8-window.h ir8-tab.h ir8-searchbox.h ir8-downloads.h \
       ir8-settings.h ir8-cellrenderer.h ir8-bookmarks.h ir8-marshal.h

OBJS = $(SRCS:.c=.o)

all: ir8

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<

ir8: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

install: ir8
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 ir8 $(DESTDIR)$(PREFIX)/bin/ir8

clean:
	rm -f ir8 $(OBJS)

.PHONY: all install clean
