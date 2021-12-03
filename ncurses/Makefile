# GNU MAKE Makefile for PDCurses demos with ncurses
#
# Usage: make [tgt] [WIDE=Y]
#
# where tgt can be any of:
# [all|demos|testcurs]...

ifeq ($(WIDE),Y)
 CFLAGS = -Wall -Wextra -Werror -pedantic -O3 -D_XOPEN_SOURCE_EXTENDED -DHAVE_NCURSESW
 LIBCURSES = -lpanelw -lncursesw
else
 CFLAGS = -Wall -Wextra -Werror -pedantic -O3 -D_XOPEN_SOURCE_EXTENDED
 LIBCURSES = -lpanel -lncurses
endif

demodir = ../demos

DEMOS    = firework init_col newtest ozdemo picsview ptest  \
    rain speed testcurs test_pan widetest worm xmas

all: $(DEMOS) tuidemo

$(DEMOS) :  %: $(demodir)/%.c
	$(CC) $(CFLAGS) -o$@ $< $(LIBCURSES)

tuidemo : $(demodir)/tuidemo.c $(demodir)/tui.c
	$(CC) $(CFLAGS) -o$@ $(demodir)/tui.c $< $(LIBCURSES)

clean:
	rm -f $(DEMOS) tuidemo
