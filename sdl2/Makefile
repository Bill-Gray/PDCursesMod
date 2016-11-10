# Makefile for PDCurses library for SDL

O = o

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR	= ..
endif
prefix		=$(DESTDIR)$(PREFIX)
exec_prefix	=$(prefix)
libdir		=$(exec_prefix)/lib
bindir		=$(exec_prefix)/bin
includedir	=$(exec_prefix)/include
docdir		=$(exec_prefix)/share/doc/pdcurses
mandir		=../doc

include $(PDCURSES_SRCDIR)/libobjs.mif

osdir		= $(PDCURSES_SRCDIR)/sdl2

PDCURSES_SDL_H	= $(osdir)/pdcsdl.h

SFLAGS		= $(shell pkg-config --cflags SDL2_ttf sdl2)
SLIBS		= $(shell pkg-config --libs SDL2_ttf sdl2)

# If your system doesn't have these, remove the defines here
SFLAGS		+= -DHAVE_VSNPRINTF -DHAVE_VSSCANF

ifeq ($(DEBUG),Y)
	CFLAGS  = -g -Wall -DPDCDEBUG
else
	CFLAGS:= $(CFLAGS)
	CFLAGS  += -O2 -Wall
	LDFLAGS:= $(LDFLAGS)
endif

ifeq ($(WIDE),Y)
	CFLAGS += -DPDC_WIDE
endif

ifeq ($(UTF8),Y)
	CFLAGS += -DPDC_FORCE_UTF8
endif

BUILD		= $(CC) $(CFLAGS) -I$(PDCURSES_SRCDIR)

ifeq ($(shell uname),Darwin)
	DEMOFLAGS = -Dmain=SDL_main
endif

LINK		= $(CC)
LDFLAGS		= $(LIBCURSES) $(SLIBS)
RANLIB		= ranlib
LIBCURSES	= libpdcurses.a

DEMOS		= firework$(EXE_EXTENSION) newdemo$(EXE_EXTENSION) ptest$(EXE_EXTENSION) rain$(EXE_EXTENSION) testcurs$(EXE_EXTENSION) tuidemo$(EXE_EXTENSION) worm$(EXE_EXTENSION) xmas$(EXE_EXTENSION) \
sdltest$(EXE_EXTENSION)

.PHONY: all libs clean demos install

all:	libs demos

libs:	$(LIBCURSES)

clean:
	-rm -rf *.o trace $(LIBCURSES) $(DEMOS)

demos:	$(DEMOS)
#	strip $(DEMOS)

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS)
	ar rv $@ $?
	-$(RANLIB) $@

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_SDL_H)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
tui.o tuidemo.o : $(PDCURSES_CURSES_H)
terminfo.o: $(TERM_HEADER)
panel.o ptest$(EXE_EXTENSION): $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

firework$(EXE_EXTENSION): $(demodir)/firework.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

newdemo$(EXE_EXTENSION): $(demodir)/newdemo.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

ptest$(EXE_EXTENSION): $(demodir)/ptest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

rain$(EXE_EXTENSION): $(demodir)/rain.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

testcurs$(EXE_EXTENSION): $(demodir)/testcurs.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

tuidemo$(EXE_EXTENSION): tuidemo.o tui.o
	$(LINK) tui.o tuidemo.o -o $@ $(LDFLAGS)

worm$(EXE_EXTENSION): $(demodir)/worm.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

xmas$(EXE_EXTENSION): $(demodir)/xmas.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

sdltest$(EXE_EXTENSION): $(osdir)/sdltest.c
	$(BUILD) $(SFLAGS) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

tui.o: $(demodir)/tui.c $(demodir)/tui.h
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tui.c

tuidemo.o: $(demodir)/tuidemo.c
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tuidemo.c

install:
	if ( test ! -d $(bindir) ) ; then mkdir -p $(bindir) ; fi
	cp $(DEMOS) $(bindir)
	if ( test ! -d $(libdir) ) ; then mkdir -p $(libdir) ; fi
	cp *.a $(libdir)
	if ( test ! -d $(includedir) ) ; then mkdir -p $(includedir) ; fi
	cp ../curses.h $(includedir)
	cp ../panel.h $(includedir)
	cp ../term.h $(includedir)
	if ( test ! -d $(docdir) ) ; then mkdir -p $(docdir) ; fi
	cp $(mandir)/*.txt $(docdir)