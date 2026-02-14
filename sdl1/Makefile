# GNU MAKE Makefile for PDCurses for SDL
#
# Usage: [g]make [DEBUG=Y] [WIDE=Y] [UTF8=Y] [LIBNAME=(name)]
#                [DLLNAME=(name)] [target]
#
# where target can be any of:
# [all|demos|libpdcurses.a|testcurs]...

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

ifeq ($(UTF8),Y)
	CFLAGS	+= -DPDC_FORCE_UTF8 -DPDC_WIDE
	SLIBS	+= -lSDL_ttf
else
	ifeq ($(WIDE),Y)
		CFLAGS	+= -DPDC_WIDE
		SLIBS	+= -lSDL_ttf
	endif
endif

ifdef CHTYPE_32
	CFLAGS += -DCHTYPE_32
endif

ifdef CHTYPE_16
	CFLAGS += -DCHTYPE_16
endif

ifeq ($(DLL),Y)
	LDFLAGS		= $(LIBCURSES_SO) $(SLIBS)
else
	LDFLAGS		= $(LIBCURSES) $(SLIBS)
endif

LIBNAME=pdcurses
DLLNAME=pdcurses

RANLIB		= ranlib
LIBCURSES	= lib$(LIBNAME).a
LIBCURSES_SO	= lib$(DLLNAME).so

BUILD		= $(CC) $(CFLAGS) -I$(PDCURSES_SRCDIR)

LINK		= $(CC)

DEMOS		+= sdltest

.PHONY: all libs clean demos tests

all:	libs

libs:	$(LIBCURSES) $(LIBCURSES_SO)

clean:
	-$(RM) *.o trace $(LIBCURSES) $(LIBCURSES_SO) $(DEMOS) $(TESTS)

demos:	$(DEMOS)
ifneq ($(DEBUG),Y)
	strip $(DEMOS)
endif

tests:	$(TESTS)
ifneq ($(DEBUG),Y)
	strip $(TESTS)
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
$(TESTS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
tui.o tuidemo.o : $(PDCURSES_CURSES_H)
terminfo.o : $(TERM_HEADER)
panel.o ptest: $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(BUILD) -c $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

$(TESTS): %: $(testdir)/%.c
	$(BUILD) $(DEMOFLAGS) -o$@ $< $(LDFLAGS)

$(DEMOS_EXCEPT_TUIDEMO): %: $(demodir)/%.c
	$(BUILD) $(DEMOFLAGS) -o$@ $< $(LDFLAGS)

tuidemo: tuidemo.o tui.o
	$(LINK) tui.o tuidemo.o -o $@ $(LDFLAGS)

sdltest: $(osdir)/sdltest.c
	$(BUILD) $(DEMOFLAGS) $(SFLAGS) -o $@ $< $(LDFLAGS)

tui.o: $(demodir)/tui.c $(demodir)/tui.h
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tui.c

tuidemo.o: $(demodir)/tuidemo.c
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tuidemo.c

include $(demodir)/nctests.mif
