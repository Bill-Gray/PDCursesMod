Miscellaneous Small Test Programs
=================================

From time to time,  small test programs for very specific bugs or features are written,  sometimes in response to an issue raised on GitHub.  They tend to be as minimal as possible,  to avoid side issues.  In almost all cases,  they will be used briefly until the bug is fixed or feature tested,  then ignored.  However,  they may be useful if further bugs/issues are found,  or as examples of how the functions are used.  Each is documented with comments at the top of the source file.

There are several that may eventually be added to this directory.  As a start,  we have :

- `ripoff.c` - code to test the `ripoffline()` and SLK functions.
- `keytest.c` - code to test the speed of keyboard input on the various platforms.  This arose from [issue #197](https://github.com/Bill-Gray/PDCursesMod/issues/197).
- `naptest.c` - code to test the `napms()` function on DOS and DOSVGA.
- `restart.c` - code to test the ability to shut Curses down completely,  do "traditional" input/output,  then restart Curses.
