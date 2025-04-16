# GNU MAKE Makefile for PDCurses library for VT
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

osdir		= $(PDCURSES_SRCDIR)/vt
common		= $(PDCURSES_SRCDIR)/common

include $(common)/libobjs.mif

RM		= rm -f

# If your system doesn't have these, remove the defines here
SFLAGS		= -DHAVE_VSNPRINTF -DHAVE_VSSCANF

CFLAGS  = -Wall -Wextra -pedantic -Werror
ifeq ($(DEBUG),Y)
	CFLAGS  += -g -DPDCDEBUG
else
	CFLAGS  += -O2
endif

ifeq ($(DOS),Y)
	CFLAGS	+= -DDOS
	E = .exe
	RM = del
else
	CFLAGS	+= -fPIC
endif

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

ifdef _w64
	PREFIX  = x86_64-w64-mingw32-
	E = .exe
endif

ifdef _w32
	PREFIX  = i686-w64-mingw32-
	E = .exe
endif

ifdef _a64
	PREFIX  = aarch64-w64-mingw32-
	E = .exe
endif

ifeq ($(OS),Windows_NT)
	E = .exe
	RM = cmd /c del
endif

LIBNAME=libpdcurses
DLLNAME=pdcurses

LIBFLAGS = rv
LIBCURSES	= $(LIBNAME).a
CC	= $(PREFIX)gcc
LIBEXE = $(PREFIX)ar
STRIP	= $(PREFIX)strip

ifeq ($(shell uname -s),FreeBSD)
	CC = cc
endif

ifeq ($(DLL),Y)
	ifeq ($(E),.exe)
		CFLAGS += -DPDC_DLL_BUILD
		LIBCURSES	= $(DLLNAME).dll
		LIBEXE = $(CC)
		LIBFLAGS = -Wl,--out-implib,$(LIBNAME).a -shared -o
	else
		ifeq ($(shell uname -s),Darwin)
			DLL_SUFFIX = .dylib
		else
			DLL_SUFFIX = .so
		endif
		LIBEXE = $(CC)
		LIBFLAGS = -shared -o
		LIBCURSES = lib$(DLLNAME)$(DLL_SUFFIX)
	endif
endif

BUILD		= $(CC) $(CFLAGS) -I$(PDCURSES_SRCDIR)

LINK		= $(CC)
LDFLAGS		= $(LIBCURSES)
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
	$(BUILD) $(SFLAGS) -c $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

$(DEMOS_EXCEPT_TUIDEMO): %$(E): $(demodir)/%.c
	$(BUILD) $(DEMOFLAGS) -o$@ $< $(LDFLAGS)

$(TESTS): %$(E): $(testdir)/%.c
	$(BUILD) $(DEMOFLAGS) -o$@ $< $(LDFLAGS)

tuidemo$(E) : tuidemo.o tui.o
	$(LINK) tui.o tuidemo.o -o $@ $(LDFLAGS)

tui.o: $(demodir)/tui.c $(demodir)/tui.h
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tui.c

tuidemo.o: $(demodir)/tuidemo.c
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tuidemo.c

configure :
	$(CC) $(CFLAGS) -o config_curses$(E) $(common)/config_curses.c
ifdef PREFIX
	wine config_curses$(E) -v -d.. $(CFLAGS)
else
	./config_curses$(E) -v -d.. $(CFLAGS)
endif
	rm config_curses$(E)

include $(demodir)/nctests.mif

install:
ifneq ($(OS),Windows_NT)
	cp lib$(DLLNAME).so /usr/local/lib
	ldconfig /usr/local/lib
endif

uninstall:
ifneq ($(OS),Windows_NT)
	$(RM) /usr/local/lib/lib$(DLLNAME).so
	ldconfig /usr/local/lib
endif
