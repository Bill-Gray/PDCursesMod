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
testdir = ../tests

DEMOS    = calendar firework init_col mbrot newtest ozdemo picsview ptest  \
    rain speed testcurs test_pan widetest worm xmas

TESTS = brk_test del_test focus ins_del keytest opaque restart ripoff

all: $(DEMOS) tuidemo

tests: $(TESTS)

$(TESTS) :  %: $(testdir)/%.c
	$(CC) $(CFLAGS) -o$@ $< $(LIBCURSES)

$(DEMOS) :  %: $(demodir)/%.c
	$(CC) $(CFLAGS) -o$@ $< $(LIBCURSES)

tuidemo : $(demodir)/tuidemo.c $(demodir)/tui.c
	$(CC) $(CFLAGS) -o$@ $(demodir)/tui.c $< $(LIBCURSES)

clean:
	rm -f $(DEMOS) tuidemo $(TESTS)
