# GNU MAKE Makefile for PDCurses library - OS/2 emx 0.9c+
#
# Usage: [g]make [-f path\Makefile] [DEBUG=Y] [DLL=Y] [target]
#
# where target can be any of:
# [all|libs|demos|pdcurses.a|pdcurses.dll|testcurs.exe...]
#

E = .exe

RM = del
CP = copy
# use following for running under 4OS/2
#RM = rm
#CP = cp

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR = ..
endif

osdir		= $(PDCURSES_SRCDIR)/os2gui
common		= $(PDCURSES_SRCDIR)/common

include $(common)/libobjs.mif

#PDCURSES_OS2_H	= $(osdir)/pdcos2.h

CC		= gcc

CFLAGS = -I$(PDCURSES_SRCDIR) -c -Wall

ifeq ($(DEBUG),Y)
	CFLAGS  += -g -DPDCDEBUG
	LDFLAGS = -g
else
	CFLAGS  += -O2
	LDFLAGS =
endif

DLLTARGET	= pdcurses.dll
DLLFLAGS 	= -Zdll -Zcrtdll -Zomf

LINK		= gcc
#EMXBIND		= emxbind
#EMXOMF		= emxomf

LIBEXE		= ar
LIBFLAGS	= rcv

ifeq ($(DLL),Y)
	O = obj
	CFLAGS += -Zdll -Zcrtdll -Zomf
	LDFLAGS += -Zomf -Zcrtdll
	LIBCURSES = pdcurses.lib
	LIBDEPS = $(LIBOBJS) $(PDCOBJS)
	PDCLIBS = $(DLLTARGET)
	EXEPOST =
	TUIPOST =
	CLEAN = *.dll *.lib
else
	O = o
#	CFLAGS += -Zomf
	LDFLAGS += -Zomf
	LIBCURSES = pdcurses.a
	LIBDEPS = $(LIBOBJS) $(PDCOBJS)
	PDCLIBS = $(LIBCURSES)
#	EXEPOST = $(EMXBIND) $* $(BINDFLAGS)
#	TUIPOST = $(EMXBIND) tuidemo $(BINDFLAGS)
	CLEAN = *.a
endif

.PHONY: all libs clean demos dist

all:	libs demos

libs:	$(PDCLIBS)

clean:
	-$(RM) *.$(O)
	-$(RM) *.def
	-$(RM) *.exe
	-$(RM) $(CLEAN)

demos:	$(DEMOS)

DEMOOBJS = calendar.$(O) firework.$(O) init_col.$(O) mbrot.$(O) newtest.$(O) \
 ozdemo.$(O) picsview.$(O) ptest.$(O) rain.$(O) speed.$(O) test_pan.$(O) testcurs.$(O) \
 tui.$(O) tuidemo.$(O) widetest.$(O) worm.$(O) xmas.$(O)

pdcurses.a : $(LIBDEPS)
	$(LIBEXE) $(LIBFLAGS) $@ $?
	-$(CP) $@ panel.a

pdcurses.lib : $(DLLTARGET)
	emximp -o $@ $*.def
	-$(CP) $@ panel.lib

$(DLLTARGET) : $(LIBDEPS)
	echo "LIBRARY PDCURSES INITINSTANCE" > $*.def
	echo "DATA MULTIPLE NONSHARED" >> $*.def
	echo "DESCRIPTION \"PDCURSES DLL for OS/2\"" >> $*.def
	echo "STACKSIZE 81920" >> $*.def
	echo "EXPORTS" >> $*.def
	emxexp $(LIBDEPS) >> $*.def
	$(LINK) $(DLLFLAGS) -o $@ $(LIBDEPS) $*.def
#	lxlite $@

$(LIBOBJS) $(PDCOBJS) $(DEMOOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_OS2_H)
$(DEMOS) : $(LIBCURSES)
panel.$(O) ptest.$(O): $(PANEL_HEADER)
terminfo.$(O): $(TERM_HEADER)

$(LIBOBJS) : %.$(O): $(srcdir)/%.c
	$(CC) -c $(CFLAGS) -o$@ $<

$(PDCOBJS) : %.$(O): $(osdir)/%.c
	$(CC) -c $(CFLAGS) -o$@ $<

$(DEMOS_EXCEPT_TUIDEMO): %.exe: %.$(O)
	echo "NAME $* WINDOWAPI" > $*.def
	$(LINK) $(LDFLAGS) -o $*$(E) $< $(LIBCURSES) $(CCLIBS) $*.def
	$(EXEPOST)

tuidemo.exe:	tuidemo.$(O) tui.$(O)
	echo "NAME tuidemo WINDOWAPI" > tuidemo.def
	$(LINK) $(LDFLAGS) -o tuidemo tuidemo.$(O) tui.$(O) $(LIBCURSES) $(CCLIBS) tuidemo.def
	$(TUIPOST)

calendar.$(O) firework.$(O) init_col.$(O) mbrot.$(O) newtest.$(O) ozdemo.$(O) picsview.$(O) \
 ptest.$(O) rain.$(O) speed.$(O) test_pan.$(O) testcurs.$(O) widetest.$(O) worm.$(O) xmas.$(O): %.$(O): \
$(demodir)/%.c
	$(CC) $(CFLAGS) -o$@ $<

tui.$(O): $(demodir)\tui.c $(demodir)\tui.h
	$(CC) $(CFLAGS) -I$(demodir) -o $@ $<

tuidemo.$(O): $(demodir)\tuidemo.c
	$(CC) $(CFLAGS) -I$(demodir) -o $@ $<
