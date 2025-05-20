PDCursesMod for X11 (new,  experimental)
========================================

This port of PDCursesMod for X11 has been written from scratch,  with
(thus far) only the key remapping array taken from pdckbd.c of the
"original" X11 port.  The only shortcoming of this version relative to
the original port is the lack of bold and italic fonts.  Once that's
fixed,  the original X11 port will be deprecated in favor of this one.

Building
--------

- Run `make`.  Add `UTF8=Y` to force the wide-character version.  Add
  `DEBUG=Y` for the debug version.  Add `NO_LEAKS=Y` to avoid Xlib's tendency
  to leak lots of memory (this does limit you to a US keyboard;  see
  `pdckbd.c`.)  Add `DLL=Y` to generate a shared library.  Add the target
  `demos` to build the sample programs,  and/or `tests` for the test programs.

- If you've built a shared library via `UTF8=Y`,  you can then run `make install`
  to install it.  Most likely,  you'll have to run as root or use `sudo`.

Usage
-----

When compiling your application, you need to include the `<curses.h>`
that comes with PDCursesMod. You also need to link your code with this
library,  of course,  and to the following libraries:

   `-lX11 -lpthread`

To do
-----

- Combining and fullwidth characters
- How to do SMB (Unicode > 0xffff)?
- Bold & italic text
- Respond to Ctrl-plus and Ctrl-minus with font size changes?

The last two should be relatively straightforward.  The first two appear
to be more challenging than I'd have expected.

Interaction with stdio
----------------------

Be aware that curses programs that expect to have a normal tty
underneath them will be very disappointed! Output directed to stdout
will go to the xterm that invoked the PDCursesMod application, or to the
console if not invoked directly from an xterm. Similarly, stdin will
expect its input from the same place as stdout.

Note that this isn't entirely a bad thing.  You can use `printf()`s for
debugging,  secure in the knowledge that they'll go to the terminal
and won't mess up the Curses screen.  You can do this for the X11
ports and GL, SDL1/2,  WinGUI,  and the OS/2 GUI ports.  You can't
with the other ports (they all do have terminals under them) or when
building with `ncurses`.

Distribution Status
-------------------

The files in this directory are released to the public domain.
