BIN=swenu
CC=gcc
OBJDIR=objs
LIBS=egl wayland-client gl wayland-egl xkbcommon
CFLAGS=-Wall -g $(shell pkg-config --cflags $(LIBS)) -I$(OBJDIR)
LDFLAGS=$(shell pkg-config --libs $(LIBS))
VPATH=/usr/share/wayland-protocols/staging/cursor-shape/:/usr/share/wayland-protocols/stable/tablet/:$(OBJDIR)
WLPROT=cursor-shape-v1.xml tablet-v2.xml
WLC=$(patsubst %.xml,$(OBJDIR)/%.c, $(WLPROT))
WLH=$(patsubst %.xml,$(OBJDIR)/%.h, $(WLPROT))
SRC=$(WLC) $(wildcard *.c) $(wildcard glad/*.c)
OBJ=$(patsubst %.c,$(OBJDIR)/%.o, $(notdir $(SRC)))

.SILENT: $(OBJ) $(WLC) $(WLH) $(BIN) $(OBJDIR) compile_flags

all: $(OBJDIR) $(BIN)

$(OBJDIR):
	[ -d $@ ] || mkdir -p $@

$(BIN): $(OBJ)
	printf "  LD %s\n" $@
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: %.c %.h
	printf "  CC %s\n" $@
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: */%.c */%.h
	printf "  CC %s\n" $@
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	printf "  CC %s\n" $@
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(OBJDIR)/%.c
$(OBJDIR)/%.c: %.xml $(OBJDIR)/%.h
	printf "  wayland-scanner %s\n" $@
	wayland-scanner private-code $<  $@

.PRECIOUS: $(OBJDIR)/%.h
$(OBJDIR)/%.h: %.xml
	printf "  wayland-scanner %s\n" $@
	wayland-scanner client-header $<  $@

compile_flags:
	echo $(CFLAGS) | tr ' ' '\n' > compile_flags.txt

clean:
	rm -rf $(BIN) $(OBJDIR)

install: all
	install -m 755 ./$(BIN) $(PREFIX)/usr/bin

uninstall:
	rm -rf $(PREFIX)/usr/bin/$(BIN)

run: all
	./$(BIN)

.PHONY: all clean run compile_flags install uninstall
