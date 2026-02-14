# GNU MAKE Makefile for PDCurses library for new X11
#
# Usage: [g]make [DEBUG=Y] [WIDE=Y] [UTF8=Y] [LIBNAME=(name)]
#                [DLLNAME=(name)] [target]
#
# where target can be any of:
# [all|demos|libpdcurses.a|testcurs]...

O = o

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR = ..
endif

osdir		= $(PDCURSES_SRCDIR)/x11new
common		= $(PDCURSES_SRCDIR)/common

include $(common)/libobjs.mif

RM		= rm -f

CFLAGS  = -Wall -Wextra -pedantic -Werror
ifeq ($(DEBUG),Y)
	CFLAGS  += -g -DPDCDEBUG
else
	CFLAGS  += -O2
endif

ifeq ($(NO_LEAKS),Y)
	CFLAGS  += -DNO_LEAKS
endif

CFLAGS	+= -fPIC

ifeq ($(UTF8),Y)
	CFLAGS	+= -DPDC_FORCE_UTF8
else
	ifeq ($(WIDE),Y)
		CFLAGS	+= -DPDC_WIDE
	endif
endif

ifdef CHTYPE_32
	CFLAGS += -DCHTYPE_32
endif

LIBNAME=libpdcurses
DLLNAME=pdcurses
XLIBS = -lX11 -lpthread

LIBFLAGS = rv
LIBCURSES	= $(LIBNAME).a
CC	= $(PREFIX)gcc
LIBEXE = $(PREFIX)ar
STRIP	= $(PREFIX)strip

ifeq ($(shell uname -s),FreeBSD)
	CC = cc
	CFLAGS += -I/usr/local/include
	LDFLAGS += -L /usr/local/lib
endif

ifeq ($(DLL),Y)
	ifeq ($(shell uname -s),Darwin)
		DLL_SUFFIX = .dylib
	else
		DLL_SUFFIX = .so
	endif
	LIBEXE = $(CC)
	LIBFLAGS = -shared -o
	LIBCURSES = lib$(DLLNAME)$(DLL_SUFFIX)
endif

BUILD		= $(CC) $(CFLAGS) -I$(PDCURSES_SRCDIR)

LINK		= $(CC)
LDFLAGS		= $(LIBCURSES) $(XLIBS)
RANLIB		= $(PREFIX)ranlib

.PHONY: all libs clean demos tests

all:	libs

libs:	$(LIBCURSES)

clean:
	-$(RM) *.o trace $(LIBCURSES) $(DEMOS) $(TESTS)

demos:	libs $(DEMOS)
ifneq ($(DEBUG),Y)
	$(STRIP) $(DEMOS)
endif

tests:	libs $(TESTS)
ifneq ($(DEBUG),Y)
	$(STRIP) $(TESTS)
endif

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS)
	$(LIBEXE) $(LIBFLAGS) $@ $?
ifeq ($(LIBCURSES),$(LIBNAME).a)
	-$(RANLIB) $@
endif

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
$(TESTS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
tui.o tuidemo.o : $(PDCURSES_CURSES_H)
terminfo.o : $(TERM_HEADER)
panel.o ptest: $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(BUILD) -c $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(BUILD) -c $<

$(DEMOS_EXCEPT_TUIDEMO): %: $(demodir)/%.c
	$(BUILD) $(DEMOFLAGS) -o$@ $< $(LDFLAGS)

$(TESTS): %: $(testdir)/%.c
	$(BUILD) $(DEMOFLAGS) -o$@ $< $(LDFLAGS)

tuidemo$(E) : tuidemo.o tui.o
	$(LINK) tui.o tuidemo.o -o $@ $(LDFLAGS)

tui.o: $(demodir)/tui.c $(demodir)/tui.h
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tui.c

tuidemo.o: $(demodir)/tuidemo.c
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tuidemo.c

configure :
	$(CC) $(CFLAGS) -o config_curses $(common)/config_curses.c
	./config_curses -v -d.. $(CFLAGS)
	rm config_curses

include $(demodir)/nctests.mif

install:
	cp lib$(DLLNAME).so /usr/local/lib
	ldconfig /usr/local/lib

uninstall:
	$(RM) /usr/local/lib/lib$(DLLNAME).so
	ldconfig /usr/local/lib
