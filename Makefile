# compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g -O2

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

SRC = src/checksums.c src/main.c src/packager.c src/unpack.c
OBJ = $(SRC:.c=.o)

all: options kpack

options:
	@echo kpack build options:
	@echo "CFLAGS	= $(CFLAGS)"
	@echo "CC	= $(CC)"

$(OBJ):
	$(CC) -c -o $@ $(CFLAGS) $(@:.o=.c)

kpack: $(OBJ)
	$(CC) -o $@ $(OBJ)
	scdoc < kpack.1.scdoc > kpack.1
	scdoc < kpack.5.scdoc > kpack.5

clean:
	rm -rf kpack kpack.1 kpack.5
	rm -rf $(OBJ)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f kpack $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	mkdir -p $(DESTDIR)$(MANPREFIX)/man5
	cp -f kpack.1 $(DESTDIR)$(MANPREFIX)/man1
	cp -f kpack.5 $(DESTDIR)$(MANPREFIX)/man5

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/kpack
	rm -f $(DESTDIR)$(MANPREFIX)/man1/kpack.1 $(DESTDIR)$(MANPREFIX)/man5/kpack.5

.PHONY: all options clean install uninstall
