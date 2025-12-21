# GNU Makefile for PDCurses - DOS VGA
#
# Usage: [g]make [-f path\Makefile] [DEBUG=Y] [WIDE=Y] [target]
#
# where target can be any of:
# [all|libs|demos|pdcurses.a|testcurs.exe...]

O = o
E = .exe
RM = del

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR = ..
endif

osdir		= $(PDCURSES_SRCDIR)/dosvga
common		= $(PDCURSES_SRCDIR)/common

include $(common)/libobjs.mif

PDCURSES_DOS_H	= $(osdir)/pdcdos.h

CC		= $(PREFIX)gcc
STRIP	= $(PREFIX)strip

CFLAGS  = -Wall -Wextra -Wshadow -Werror -pedantic
ifeq ($(DEBUG),Y)
	CFLAGS  += -g -DPDCDEBUG
	LDFLAGS = -g
else
	CFLAGS  += -O2
	LDFLAGS =
endif

CFLAGS += -I$(PDCURSES_SRCDIR)

ifeq ($(WIDE),Y)
	CFLAGS += -DPDC_WIDE
endif

LINK		= $(PREFIX)gcc

LIBEXE		= $(PREFIX)ar
LIBFLAGS	= rcv

LIBCURSES	= pdcurses.a

.PHONY: all libs clean demos dist

all:	libs

libs:	$(LIBCURSES)

clean:
	-$(RM) *.o
	-$(RM) *.a
	-$(RM) *.exe

demos:	$(DEMOS)
ifneq ($(DEBUG),Y)
	$(STRIP) *.exe
endif

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS)
	$(LIBEXE) $(LIBFLAGS)S $@ addch.o addchstr.o addstr.o attr.o beep.o bkgd.o border.o clear.o color.o debug.o delch.o deleteln.o
	$(LIBEXE) $(LIBFLAGS)S $@ getch.o getstr.o getyx.o inch.o inchstr.o initscr.o inopts.o insch.o insstr.o instr.o kernel.o keyname.o
	$(LIBEXE) $(LIBFLAGS)S $@ mouse.o move.o outopts.o overlay.o pad.o panel.o pdcclip.o pdcdisp.o pdcgetsc.o pdckbd.o pdcscrn.o pdcsetsc.o
	$(LIBEXE) $(LIBFLAGS)  $@ pdcutil.o printw.o refresh.o scanw.o scr_dump.o scroll.o slk.o termattr.o terminfo.o touch.o util.o window.o

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_DOS_H)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
panel.o : $(PANEL_HEADER)
terminfo.o : $(TERM_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(CC) -c $(CFLAGS) $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(CC) -c $(CFLAGS) $<

firework.exe ozdemo.exe rain.exe testcurs.exe worm.exe xmas.exe \
ptest.exe: %.exe: $(demodir)/%.c
	$(CC) $(CFLAGS) -o$@ $< $(LIBCURSES)

tuidemo.exe: tuidemo.o tui.o
	$(LINK) $(LDFLAGS) -o$@ tuidemo.o tui.o $(LIBCURSES)

tui.o: $(demodir)/tui.c $(demodir)/tui.h $(PDCURSES_CURSES_H)
	$(CC) -c $(CFLAGS) -I$(demodir) -o$@ $<

tuidemo.o: $(demodir)/tuidemo.c $(PDCURSES_CURSES_H)
	$(CC) -c $(CFLAGS) -I$(demodir) -o$@ $<
