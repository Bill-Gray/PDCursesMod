Miscellaneous Small Test Programs
=================================

From time to time,  small test programs for very specific bugs or features are written,  sometimes in response to an issue raised on GitHub.  They tend to be as minimal as possible,  to avoid side issues.  In almost all cases,  they will be used briefly until the bug is fixed or feature tested,  then ignored.  However,  they may be useful if further bugs/issues are found,  or as examples of how the functions are used.  Each is documented with comments at the top of the source file.

There are several that may eventually be added to this directory.  As a start,  we have :

- `brk_test.c` - test to make sure that Ctrl-C stops the program in `noraw()` mode,  and does not stop it in `raw()` mode.
- `del_test.c` - test to explore some problems in deleting windows that have subwindows.
- `focus.c` - example use of `define_key()`,  and checking out ability to detect focus changes in the console.
- `ins_del.c`- tests `wscrl()`, `winsertln()`, `wdeleteln()`,  and `winsdelln()`
- `keytest.c` - tests the speed of keyboard input on the various platforms.  This arose from [issue #197](https://github.com/Bill-Gray/PDCursesMod/issues/197).
- `naptest.c` - tests the `napms()` function on DOS and DOSVGA.
- `opaque.c` - tests some relatively new functions to access `SCREEN` elements.
- `restart.c` - tests the ability to shut Curses down completely,  do "traditional" input/output,  then restart Curses.
- `ripoff.c` - tests the `ripoffline()` and SLK functions.
