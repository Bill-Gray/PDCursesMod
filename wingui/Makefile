# GNU MAKE Makefile for PDCurses library - WIN32/64/ARM MinGW GCC/LLVM
#
# Usage: [g]make [-f path\Makefile] [DEBUG=Y] [DLL=Y] [WIDE=Y] [UTF8=Y]
#        [LIBNAME=(name)] [DLLNAME=(name)] [target]
#        [_w32=Y | _w64=Y | _a64=Y ]
#
# where target can be any of:
# [all|demos|pdcurses.a|testcurs.exe...]

O = o
E = .exe

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR = ..
endif

osdir		= $(PDCURSES_SRCDIR)/wingui
common		= $(PDCURSES_SRCDIR)/common

include $(common)/libobjs.mif

uname_S := $(shell uname -s 2>/dev/null)

CAT		= cat
PREFIX		=
PATH_SEP		= /
CP		= cp
RM		= rm -f

# It appears we have three cases:  we're running in Cygwin/MSYS;  or we're
# running in command.com on Windows with MinGW;  or we're on Linux or BSD
# or similar system,  cross-compiling with MinGW.

ifneq (,$(findstring CYGWIN,$(uname_S)))
	#  Insert Cygwin-specific changes here
	ON_WINDOWS = 1
endif
ifneq (,$(findstring MINGW32_NT,$(uname_S)))
	#  Insert MINGW32-specific changes here
	ON_WINDOWS = 1
endif
ifneq (,$(findstring MINGW64_NT,$(uname_S)))
	#  Insert MINGW64-specific changes here
	ON_WINDOWS = 1
endif

ifeq ($(uname_S),)
	CAT = type
	PATH_SEP = \\
	CP = copy
	RM = cmd /c del
	ON_WINDOWS = 1
endif

# If we aren't on Windows,  assume MinGW on a Linux-like host
# Only decision is:  are we doing a 64-bit compile (_w64 defined)?

ifndef ON_WINDOWS
	PREFIX  = i686-w64-mingw32-
	ifdef _w64
		PREFIX  = x86_64-w64-mingw32-
	endif
	ifdef _a64
		PREFIX  = aarch64-w64-mingw32-
	endif
endif

PDCURSES_WIN_H	= $(osdir)/pdcwin.h

CC	= $(PREFIX)gcc

AR	= $(PREFIX)ar
STRIP	= $(PREFIX)strip

CFLAGS  += -Wall -Wextra -pedantic
ifeq ($(DEBUG),Y)
	CFLAGS  += -g -DPDCDEBUG
	LDFLAGS = -g
else
	CFLAGS  += -O3
	LDFLAGS =
endif

CFLAGS += -I$(PDCURSES_SRCDIR)

ifdef CHTYPE_32
	CFLAGS += -DCHTYPE_32
endif

ifeq ($(UTF8),Y)
	CFLAGS	+= -DPDC_FORCE_UTF8
else
	ifeq ($(WIDE),Y)
		CFLAGS	+= -DPDC_WIDE
	endif
endif

LINK	   = $(CC)
LIBNAME = pdcurses
DLLNAME = pdcurses

ifeq ($(DLL),Y)
	CFLAGS += -DPDC_DLL_BUILD
	LIBEXE = $(CC)
	LIBFLAGS = -Wl,--out-implib,$(LIBNAME).a -static-libgcc -shared -o
	LIBCURSES = $(DLLNAME).dll
	LIBDEPS = $(LIBOBJS) $(PDCOBJS)
	LIBSADDED = -lgdi32 -lcomdlg32 -lwinmm
	EXELIBS =
	CLEAN = $(LIBCURSES) *.a
else
	LIBEXE = $(AR)
ifeq ($(PREFIX),)
	LIBFLAGS = rcv
else
	LIBFLAGS = rv
endif
	LIBCURSES = $(LIBNAME).a
	LIBDEPS = $(LIBOBJS) $(PDCOBJS)
	LIBSADDED =
	EXELIBS = -lgdi32 -lcomdlg32 -lwinmm
	CLEAN = *.a
endif

.PHONY: all libs clean demos dist

all:	libs

libs:	$(LIBCURSES)

clean:
	-$(RM) *.o
	-$(RM) *.exe
	-$(RM) *.dll
	-$(RM) $(CLEAN)

demos:	$(DEMOS)
ifneq ($(DEBUG),Y)
	$(STRIP) *.exe
endif

$(LIBCURSES) : $(LIBDEPS)
	$(LIBEXE) $(LIBFLAGS) $@ $? $(LIBSADDED)
	$(CP) $(LIBNAME).a panel.a

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_WIN_H)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
panel.o : $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(CC) -c $(CFLAGS) $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(CC) -c $(CFLAGS) $<

$(DEMOS_EXCEPT_TUIDEMO): %.exe: $(demodir)/%.c
	$(CC) $(CFLAGS) -mwindows -o$@ $< $(LIBCURSES) $(EXELIBS)

tuidemo.exe: tuidemo.o tui.o
	$(LINK) $(LDFLAGS) -mwindows -o$@ tuidemo.o tui.o $(LIBCURSES) $(EXELIBS)

tui.o: $(demodir)/tui.c $(demodir)/tui.h $(PDCURSES_CURSES_H)
	$(CC) -c $(CFLAGS) -I$(demodir) -o$@ $<

tuidemo.o: $(demodir)/tuidemo.c $(PDCURSES_CURSES_H)
	$(CC) -c $(CFLAGS) -I$(demodir) -o$@ $<

configure :
	$(CC) $(CFLAGS) -o config_curses$(E) $(common)/config_curses.c
ifdef ON_WINDOWS
	config_curses$(E) -v -d.. $(CFLAGS)
else
	wine config_curses$(E) -v -d.. $(CFLAGS)
endif
	rm config_curses$(E)

include $(PDCURSES_SRCDIR)/demos/nctests.mif
