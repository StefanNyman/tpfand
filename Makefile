VERSION = $(shell git describe)
BINARY = tpfand
SRCDIR = src
BUILDDIR = build
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic -fstack-protector-all -O2 -Wl,-z,relro,-z,now $(DEBUG) -DVERSION=\"$(VERSION)\" -DBINARY=\"$(BINARY)\" -D_GNU_SOURCE -D_FORTIFY_SOURCE=2
LDFLAGS = 
SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(addprefix $(BUILDDIR)/,$(SRC:%.c=%.o))
DEPS := $(patsubst %.o,%.d,$(OBJ))

all: $(BINARY)

$(BINARY): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $(BINARY)

-include $(DEPS)

$(BUILDDIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(dir $<) -MMD -MP -c $< -o $@

debug:
	$(MAKE) $(MAKEFILE) DEBUG="-g -DDEBUG"

strip:
	strip $(BINARY)

install: $(BINARY) strip
	install -D -m755 $(BINARY) $(DESTDIR)$(PREFIX)/bin/$(BINARY)
	install -D -m644 $(BINARY).conf $(DESTDIR)$(PREFIX)/share/doc/$(BINARY)/$(BINARY).conf
	install -D -m644 LICENSE $(DESTDIR)/$(PREFIX)/share/licenses/$(BINARY)-git/LICENSE
	install -D -m755 tpfand.service $(DESTDIR)/$(PREFIX)/lib/systemd/system/tpfand.service

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/$(BINARY)
	rm -f $(DESTDIR)/$(PREFIX)/share/doc/$(BINARY)/$(BINARY).conf
	rm -rf $(DESTDIR)/$(PREFIX)/share/licenses/$(BINARY)-git
	rm $(DESTDIR)/$(PREFIX)/lib/systemd/system/tpfand.service

clean:
	rm -rf $(BUILDDIR)
	rm -f $(BINARY) 

check:

.PHONY: install uninstall clean check


