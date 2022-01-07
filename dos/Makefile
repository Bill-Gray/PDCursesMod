# GNU Makefile for PDCurses - DOS
#
# Usage: [g]make [-f path\Makefile] [DEBUG=Y] [target]
#
# where target can be any of:
# [all|libs|demos|pdcurses.a|testcurs.exe...]

O = o
E = .exe
RM = del

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR = ..
endif

osdir		= $(PDCURSES_SRCDIR)/dos
common		= $(PDCURSES_SRCDIR)/common

include $(common)/libobjs.mif

PDCURSES_DOS_H	= $(osdir)/pdcdos.h

CC		= gcc
STRIP	= strip

CFLAGS  = -Wall
ifeq ($(DEBUG),Y)
	CFLAGS  += -g -DPDCDEBUG
	LDFLAGS = -g
else
	CFLAGS  += -O2
	LDFLAGS =
endif

CFLAGS += -I$(PDCURSES_SRCDIR)

LINK		= gcc

LIBEXE		= ar
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
	$(LIBEXE) $(LIBFLAGS) $@ $?

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_DOS_H)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
panel.o : $(PANEL_HEADER)

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
