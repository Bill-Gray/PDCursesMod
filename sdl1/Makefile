# Makefile for PDCurses for SDL

O = o

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR	= ..
endif

osdir		= $(PDCURSES_SRCDIR)/sdl1
include $(PDCURSES_SRCDIR)/common/libobjs.mif

SFLAGS		= $(shell sdl-config --cflags)
SLIBS		= $(shell sdl-config --libs)

CC		= gcc
RM		= rm -f

PDCURSES_SDL_H	= $(osdir)/pdcsdl.h

CFLAGS  = -Wall -Wextra -Werror -pedantic -fPIC
ifeq ($(DEBUG),Y)
	CFLAGS  += -g -DPDCDEBUG
else
	CFLAGS  += -O2
endif

ifeq ($(WIDE),Y)
	CFLAGS	+= -DPDC_WIDE
	SLIBS	+= -lSDL_ttf
endif

ifeq ($(UTF8),Y)
	CFLAGS	+= -DPDC_FORCE_UTF8
endif

ifdef CHTYPE_32
	CFLAGS += -DCHTYPE_32
endif
ifeq ($(DLL),Y)
	LDFLAGS		= $(LIBCURSES_SO) $(SLIBS)
else
	LDFLAGS		= $(LIBCURSES) $(SLIBS)
endif

RANLIB		= ranlib
LIBCURSES	= libpdcurses.a
LIBCURSES_SO	= libpdcurses.so

BUILD		= $(CC) $(CFLAGS) -I$(PDCURSES_SRCDIR)

LINK		= $(CC)

DEMOS		+= sdltest

.PHONY: all libs clean demos

all:	libs

libs:	$(LIBCURSES) $(LIBCURSES_SO)

clean:
	-$(RM) *.o trace $(LIBCURSES) $(LIBCURSES_SO) $(DEMOS)

demos:	$(DEMOS)
ifneq ($(DEBUG),Y)
	strip $(DEMOS)
endif

install:
ifneq ($(OS),Windows_NT)
	cp $(LIBCURSES_SO) /usr/local/lib
	ldconfig /usr/local/lib
endif

uninstall:
ifneq ($(OS),Windows_NT)
	rm /usr/local/lib/$(LIBCURSES_SO)
	ldconfig /usr/local/lib
endif

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS)
	ar rv $@ $?
	-$(RANLIB) $@

$(LIBCURSES_SO) : $(LIBOBJS) $(PDCOBJS)
	$(CC) -shared -o $@ $? $(SLIBS)

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_SDL_H)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
tui.o tuidemo.o : $(PDCURSES_CURSES_H)
panel.o ptest: $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(BUILD) -c $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

firework: $(demodir)/firework.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

init_col: $(demodir)/init_col.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

newtest: $(demodir)/newtest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

ozdemo: $(demodir)/ozdemo.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

picsview: $(demodir)/picsview.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

ptest: $(demodir)/ptest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

rain: $(demodir)/rain.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

speed: $(demodir)/speed.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

testcurs: $(demodir)/testcurs.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

test_pan: $(demodir)/test_pan.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

tuidemo: tuidemo.o tui.o
	$(LINK) tui.o tuidemo.o -o $@ $(LDFLAGS)

worm: $(demodir)/worm.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

xmas: $(demodir)/xmas.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

sdltest: $(osdir)/sdltest.c
	$(BUILD) $(DEMOFLAGS) $(SFLAGS) -o $@ $< $(LDFLAGS)

tui.o: $(demodir)/tui.c $(demodir)/tui.h
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tui.c

tuidemo.o: $(demodir)/tuidemo.c
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tuidemo.c

include $(demodir)/nctests.mif
