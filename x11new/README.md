PDCurses for X11 (new,  experimental)
=====================================

This port of PDCurses for X11 has been written from scratch,  with
(thus far) only the key remapping array taken from pdckbd.c of the
"original" X11 port.  It is,  as yet,  unclear if this will be a
replacement for the original port.  As noted in the 'to do' section,
it's early days for this port (no known bugs,  but many unimplemented
and unfinished bits of functionality).

Building
--------

- Run "make".  Add UTF8=Y to force the wide-character version.  Add
  DEBUG=Y for the debug version.  Add the target "demos" to build
  the sample programs,  and/or "tests" for the test programs.

- Currently,  there is no "make install".

Usage
-----

When compiling your application, you need to include the \<curses.h\>
that comes with PDCurses. You also need to link your code with
libXCurses. You will need to link with the following libraries:

   -lX11 -lpthread

To do
-----

- Add mouse handling
- Combining and fullwidth characters
- How to do SMB (Unicode > 0xffff)?
- Blinking text
- Under/over/strikeout/left/right lines
- Bold & italic text
- Respond to Ctrl-plus and Ctrl-minus with font size changes?
- Programmatic resizing?

Most of these shouldn't be all that tough to do.

Interaction with stdio
----------------------

Be aware that curses programs that expect to have a normal tty
underneath them will be very disappointed! Output directed to stdout
will go to the xterm that invoked the PDCurses application, or to the
console if not invoked directly from an xterm. Similarly, stdin will
expect its input from the same place as stdout.

Distribution Status
-------------------

The files in this directory are released to the public domain.
