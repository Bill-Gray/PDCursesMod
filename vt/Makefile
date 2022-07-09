# GNU MAKE Makefile for PDCurses library for VT
#
# Usage: [g]make [DEBUG=Y] [WIDE=Y] [UTF8=Y] [target]
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

ifeq ($(OS),Windows_NT)
	E = .exe
	RM = cmd /c del
endif

LIBFLAGS = rv
LIBCURSES	= libpdcurses.a
CC	= $(PREFIX)gcc
LIBEXE = $(PREFIX)ar
STRIP	= $(PREFIX)strip

ifeq ($(DLL),Y)
	ifeq ($(E),.exe)
		CFLAGS += -DPDC_DLL_BUILD
		LIBCURSES	= pdcurses.dll
		LIBEXE = $(CC)
		LIBFLAGS = -Wl,--out-implib,pdcurses.a -shared -o
	else
		ifeq ($(shell uname -s),Darwin)
			DLL_SUFFIX = .dylib
		else
			DLL_SUFFIX = .so
		endif
		LIBEXE = $(CC)
		LIBFLAGS = -shared -o
		LIBCURSES = libpdcurses$(DLL_SUFFIX)
	endif
endif

BUILD		= $(CC) $(CFLAGS) -I$(PDCURSES_SRCDIR)

LINK		= $(CC)
LDFLAGS		= $(LIBCURSES)
RANLIB		= ranlib

.PHONY: all libs clean demos

all:	libs

libs:	$(LIBCURSES)

clean:
	-$(RM) *.o trace $(LIBCURSES) $(DEMOS)

demos:	libs $(DEMOS)
ifneq ($(DEBUG),Y)
	$(STRIP) $(DEMOS)
endif

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS)
	$(LIBEXE) $(LIBFLAGS) $@ $?
ifeq ($(LIBCURSES),libpdcurses.a)
	-$(RANLIB) $@
endif

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
tui.o tuidemo.o : $(PDCURSES_CURSES_H)
panel.o ptest: $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

calendar$(E) : $(demodir)/calendar.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

firework$(E) : $(demodir)/firework.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

init_col$(E) : $(demodir)/init_col.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

mbrot$(E) : $(demodir)/mbrot.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

ozdemo$(E) : $(demodir)/ozdemo.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

newtest$(E) : $(demodir)/newtest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

picsview$(E) : $(demodir)/picsview.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

ptest$(E) : $(demodir)/ptest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

rain$(E) : $(demodir)/rain.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

speed$(E) : $(demodir)/speed.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

testcurs$(E) : $(demodir)/testcurs.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

test_pan$(E) : $(demodir)/test_pan.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

tuidemo$(E) : tuidemo.o tui.o
	$(LINK) tui.o tuidemo.o -o $@ $(LDFLAGS)

widetest$(E) : $(demodir)/widetest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

worm$(E) : $(demodir)/worm.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

xmas$(E) : $(demodir)/xmas.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

tui.o: $(demodir)/tui.c $(demodir)/tui.h
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tui.c

tuidemo.o: $(demodir)/tuidemo.c
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tuidemo.c

include $(demodir)/nctests.mif

install:
ifneq ($(OS),Windows_NT)
	cp libpdcurses.so /usr/local/lib
	ldconfig /usr/local/lib
endif

uninstall:
ifneq ($(OS),Windows_NT)
	$(RM) /usr/local/lib/libpdcurses.so
	ldconfig /usr/local/lib
endif
