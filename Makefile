VERSION = $(shell git describe)
BINARY = tpfand
SRCDIR = src
BUILDDIR = build
CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -Wfatal-errors -pedantic -fstack-protector-all -O2 $(DEBUG) -DVERSION=\"$(VERSION)\" -DBINARY=\"$(BINARY)\" -D_GNU_SOURCE
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

install: $(BINARY)
	intall -D -m755 $(BINARY) $(DESTDIR)$(PREFIX)/bin/$(BINARY)
	install -D -m644 $(BINARY).conf $(DESTDIR)$(PREFIX)/share/doc/$(BINARY)/$(BINARY).conf

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/$(BINARY)
	rm -f $(DESTDIR)/$(PREFIX)/share/doc/$(BINARY)/$(BINARY).conf

clean:
	rm -rf $(BUILDDIR)
	rm -f $(BINARY) 

.PHONY: install uninstall clean


