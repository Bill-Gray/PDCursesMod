PDCurses for VT
================

This directory contains source code to support PDCurses using a mix of
xterm,  VT-100,  VT-200,  and ANSI escape sequences to set colors,  position
the cursor,  etc.  Note that the name is misleading;  it uses an olio of
control sequences from

https://www.gnu.org/software/screen/manual/html_node/Control-Sequences.html

which may,  or may not,  work on your terminal.  I've tested it in urxvt,
xterm, and QTerminal on Linux and FreeBSD,  and in `cmd` on Win10 and (with
NANSI.COM or NANSI.SYS)  on Windows ME.  MS-DOS (again with NANSI) and
Linux console modes 'sort of' work (no mouse and the colors need work).

It assumes that 256 colors are available (16 in NANSI mode),  but it can use
full RGB on terminals that support it.

Building
--------

In GNU/Linux,  run `make` or `make WIDE=Y` or `make UTF8=Y`.  You can add `-w64` or `-w32`
to cross-compile 64-bit or 32-bit Windows executables,  using MinGW64.
(But see warnings below about Windows.)
Add `DLL=Y` to get a DLL for Windows builds,  or a shared library (.so)
on *nix builds.  Run `make install` (you'll probably need to be root for
this) to install the shared library.

In *BSD,  use `gmake` or `gmake WIDE=Y`.  Cross-compiling to Windows
should be possible there as well.

For DOS/Windows,  makefiles for Borland,  Digital Mars,  MSVC, and
OpenWATCOM are provided,  but are at the 'it works on my machine' stage,
and haven't really been thoroughly vetted by others yet.

Building for Emscripten / the browser
-------------------------------------

The VT port also builds under Emscripten and renders into an xterm.js
terminal,  or any other terminal emulator driven through a PTY-style
JavaScript bridge such as xterm-pty.  Build the library with the wide
character API and a 32-bit chtype:

```sh
mkdir build && cd build
emcc -O2 -std=gnu99 -D_GNU_SOURCE -DPDC_WIDE -DCHTYPE_32 \
     -I.. -c ../pdcurses/*.c ../vt/*.c
emar rcs libpdcurses.a *.o
```

The 32-bit chtype matters for programs that keep color pairs in an `int`
(Angband's attribute tables,  for example);  the default 64-bit chtype does
not fit there.  With `PDC_WIDE`,  wide strings are written as UTF-8,  which
is what the browser expects.

Link applications with Asyncify enabled,  because `PDC_napms()` hands its
wait to the browser event loop instead of blocking it:

```sh
emcc app.c libpdcurses.a -sASYNCIFY=1 -o app.js
```

At run time,  the terminal size is read from the PTY with `TIOCGWINSZ`,  so
the program sees the size the xterm.js terminal had at startup.  Setting
`TERM=xterm-256color` gives the full 256-color palette;  `PDC_VT=RGB`
requests true color,  as on a native terminal.

Some POSIX facilities have no browser equivalent:

- Forking a sound player for the bell:  the port just flashes the screen.
- `SIGWINCH`:  a resize of the xterm.js window is not delivered to the
  program,  so the geometry stays fixed for its lifetime.
- Clipboard functions,  which are absent on this platform anyway.
- The terminfo and termcap functions remain the usual stubs;  the VT port
  uses fixed ANSI sequences.

Caveats
-------

VT-style sequences are partially supported in Windows 10 and 11.  However,
you get 16 colors (no RGB),  the mouse does not necessarily work,  and the
console size cannot be determined or set.  Use of the VT platform in Windows
is therefore strongly discouraged;  use WinGUI or WinCon instead.

This code will usually figure out the capabilities of the underlying terminal.
It errs on the conservative side and may not recognize what your terminal
can actually do.  If it doesn't think full RGB coloring is supported,
then RGB colors will be remapped to the 6x6x6 color cube.  If that happens
(resulting in slightly ugly coloring),  you can tell PDCurses you
really do have true color by setting

`PDC_VT=RGB`

`export PDC_VT`

Note that you can explain to PDCurses more capabilities of the terminal,  e.g.,

`PDC_VT=RGB UNDERLINE BLINK DIM STANDOUT`

to say that the underlying terminal supports true-color,  underlined,
blinking,  dimmed,  and 'standout' text.  (The Right Thing to Do here
would be to dig around in the terminfo database,  as ncurses does,  both
to know the control sequences to use and the actual capabilities of
the terminal.)

Arrow keys and some function keys are recognized (see the `tbl` array
in `pdckey.c`).  Some mouse input is recognized.  Shift,  Ctrl,  and Alt
function keys and arrows are (mostly) not correctly identified;  I've
not figured out how those keys are supposed to be detected yet.  Or if
they can be.  None of the 'extended' keys found on some keyboards,  such
as Browser Back/Forward,  Search,  Refresh,  Stop,  etc.,  are detected,
on any platform.

Clipboard functions are currently completely absent on this platform.  I
expect to be able to add clipboard functions for Windows by recycling
code from the Windows GUI and console flavors,  and have clipboard
access code for X11 that is not yet included here.

Distribution Status
-------------------

The files in this directory are released to the Public Domain.

