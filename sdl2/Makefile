# Makefile for PDCurses for SDL2

O = o

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR	= ..
endif

osdir		= $(PDCURSES_SRCDIR)/sdl2
common		= $(PDCURSES_SRCDIR)/common

include $(common)/libobjs.mif

ifdef _w64
	PREFIX=x86_64-w64-mingw32-
	E = .exe
endif
ifdef _w32
	PREFIX=i686-w64-mingw32-
	E = .exe
endif

ifeq ($(OS),Windows_NT)
	E = .exe
	RM = cmd /c del

	include $(osdir)/versions.mif

	PLATFORM = $(shell $(CC) -dumpmachine)

	SFLAGS = -I$(SDLBASE)/$(PLATFORM)/include/SDL2
	SLIBS = -L$(SDLBASE)/$(PLATFORM)/lib -lSDL2main -lSDL2

	TFLAGS = -I$(TTFBASE)/$(PLATFORM)/include/SDL2
	TLIBS =-L$(TTFBASE)/$(PLATFORM)/lib -lSDL2_ttf

	DEMOFLAGS = -mwindows
else
	RM = rm -f

	SFLAGS = $(shell sdl2-config --cflags)
	SLIBS = $(shell sdl2-config --libs)

	TLIBS = -lSDL2_ttf
endif

CC		= $(PREFIX)gcc
WINDRES		= windres

PDCURSES_SDL_H	= $(osdir)/pdcsdl.h

CFLAGS  = -Wall -Wextra -Werror -pedantic -fPIC
ifeq ($(DEBUG),Y)
	CFLAGS  += -g -DPDCDEBUG
else
	CFLAGS  += -O2
endif

ifeq ($(WIDE),Y)
	CFLAGS += -DPDC_WIDE
	SFLAGS += $(TFLAGS)
	SLIBS  += $(TLIBS)
endif

ifeq ($(UTF8),Y)
	CFLAGS += -DPDC_FORCE_UTF8
endif

ifdef CHTYPE_32
	CFLAGS += -DCHTYPE_32
endif

ifeq ($(DLL),Y)
	ifeq ($(OS),Windows_NT)
		CFLAGS += -DPDC_DLL_BUILD
		LIBEXE = $(CC)
		LIBFLAGS = -Wl,--out-implib,libpdcurses.a -shared -o
		LIBCURSES = pdcurses.dll
		CLEAN = $(LIBCURSES) *.a
		RESOURCE = pdcurses.o
		LIBLIBS = $(SLIBS)
		LDFLAGS = $(LIBCURSES)
	else
		ifeq ($(shell uname -s),Darwin)
			DLL_SUFFIX = .dylib
		else
			DLL_SUFFIX = .so
		endif
		LIBEXE = $(CC)
		LIBFLAGS = -shared -o
		LIBCURSES = libpdcurses$(DLL_SUFFIX)
		LIBLIBS = $(SLIBS)
		LDFLAGS = $(LIBCURSES)
		CLEAN = *$(DLL_SUFFIX)
	endif
else
	LIBEXE = $(AR)
	LIBFLAGS = rcv
	LIBCURSES = libpdcurses.a
	LDFLAGS = $(LIBCURSES) $(SLIBS)
	CLEAN = *.a
endif

BUILD		= $(CC) $(CFLAGS) -I$(PDCURSES_SRCDIR)

LINK		= $(CC)

DEMOS		+= sdltest$(E)

.PHONY: all libs clean demos

all:	libs

libs:	$(LIBCURSES)

clean:
	-$(RM) *.o trace $(CLEAN) $(DEMOS)

demos:	$(DEMOS)
ifneq ($(DEBUG),Y)
	strip $(DEMOS)
endif

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

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS) $(RESOURCE)
	$(LIBEXE) $(LIBFLAGS) $@ $? $(LIBLIBS)

pdcurses.o: $(common)/pdcurses.rc
	$(WINDRES) -i $(common)/pdcurses.rc pdcurses.o

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_SDL_H)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
tui.o tuidemo.o : $(PDCURSES_CURSES_H)
panel.o ptest$(E) test_pan$(E): $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(BUILD) -c $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(BUILD) $(SFLAGS) -c $<

calendar$(E): $(demodir)/calendar.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

firework$(E): $(demodir)/firework.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

init_col$(E): $(demodir)/init_col.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

mbrot$(E): $(demodir)/mbrot.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

ozdemo$(E): $(demodir)/ozdemo.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

picsview$(E): $(demodir)/picsview.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

ptest$(E): $(demodir)/ptest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

newtest$(E): $(demodir)/newtest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

rain$(E): $(demodir)/rain.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

speed$(E): $(demodir)/speed.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

testcurs$(E): $(demodir)/testcurs.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

test_pan$(E): $(demodir)/test_pan.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

tuidemo$(E): tuidemo.o tui.o
	$(LINK) tui.o tuidemo.o -o $@ $(LDFLAGS)

worm$(E): $(demodir)/worm.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

xmas$(E): $(demodir)/xmas.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

sdltest$(E): $(osdir)/sdltest.c
	$(BUILD) $(SFLAGS) $(DEMOFLAGS) -o $@ $< $(LIBCURSES) $(SLIBS)

tui.o: $(demodir)/tui.c $(demodir)/tui.h
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tui.c

tuidemo.o: $(demodir)/tuidemo.c
	$(BUILD) -c $(DEMOFLAGS) $(demodir)/tuidemo.c

widetest$(E): $(demodir)/widetest.c
	$(BUILD) $(DEMOFLAGS) -o $@ $< $(LDFLAGS)

include $(demodir)/nctests.mif
