PDCurses for OpenGL 3.3
=======================

This is a port of PDCurses for OpenGL 3.3 on top of SDL2. It's based on the
SDL2 port, but uses OpenGL for character drawing.


Building
--------

- Depends on both SDL2 and SDL2_ttf, always.

- On \*nix (including Linux and Mac OS X), run "make" in the gl
  directory. There is no configure script (yet?) for this port. This
  assumes a working sdl-config, and GNU make. It builds the library
  pdcurses.a (or pdcurses.so/pdcurses.dylib with DLL=Y).

- With MinGW, edit the Makefile to point to the appropriate include and
  library paths, and then run "mingw32-make".

- With MSVC, edit Makefile.vc if needed, and do "nmake -f Makefile.vc".

- The makefile recognizes the optional PDCURSES_SRCDIR environment variable,
  and the option "DEBUG=Y", as with the console ports. "WIDE=Y" builds a
  version that uses 32-bit Unicode characters. "UTF8=Y" makes PDCurses ignore
  the system locale, and treat all narrow-character strings as UTF-8; this
  option has no effect unless WIDE=Y is also set. You can specify "DLL=Y" to
  build a dynamic rather than static library. The dynamic library is called
  pdcurses.dll, pdcurses.so, or pdcurses.dylib on Windows, Linux, or Mac OS X
  respectively.  And on all platforms, add the target "demos" to build the
  sample programs.


Usage
-----

There are no special requirements to use PDCurses for OpenGL -- all
PDCurses-compatible code should work fine.

Unlike the SDL2 PDCurses, this port assumes that the user isn't doing any SDL
or OpenGL stuff behind it's back, as that would break our assumptions of the
OpenGL state machine. However, you can change the SDL window parameters safely
(size, fullscreen).

The font can be set via the environment variable PDC_FONT. If defined, it must
point to a TrueType font. Only true monospaced fonts work well. The font can be
set at compile time via PDC_FONT_PATH, and/or at runtime via pdc_ttffont. The
environment variable PDC_FONT_SIZE is also available to control the font size
(also as a compile-time define, and at runtime as pdc_font_size.) The character
mapping for chtypes is UTF-32. However, with SDL2_ttf versions older than
2.0.18, only the Basic Multilingual Plane characters are available.

The default font (if not redefined) is based on the OS:

- Windows: C:/Windows/Fonts/consola.ttf

- Mac: /Library/Fonts/Menlo.ttc

- Other: /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf

Icons
-----

The icon (used with SDL_SetWindowIcon() -- not used for the executable
file) can be set via the environment variable PDC_ICON, and falls back
to "pdcicon.bmp", and then to the built-in icon from iconbmp.h. The
built-in icon is the PDCurses logo, as seen in ../common/icon32.xpm.

If pdc_screen is preinitialized (see below), PDCurses does not attempt
to set the icon.

Screen size
-----------

The default screen size is 80x25 characters (whatever size they may be),
but you can override this via the environment variables PDC_COLS and/or
PDC_LINES.

You can change what the window scaling does with these two variables:
```
PDCEX int pdc_resize_mode;
PDCEX int pdc_interpolation_mode;
```

`pdc_resize_mode` has four options: `PDC_GL_RESIZE_NORMAL` makes resizing change
the number of visible characters on screen. `PDC_GL_RESIZE_STRETCH` stretches
the existing LINES & COLS to fit the window size. `PDC_GL_RESIZE_SCALE` does
the same, but keeps aspect ratio by adding black bars. `PDC_GL_RESIZE_INTEGER`
changes scaling in integer steps.

Alternatively,  one can set the environment variable PDC_RESIZE to 0, 1, 2, or
3 to get the four options listed above.

`pdc_interpolation_mode` can be used to change between nearest-neighbor
(`PDC_GL_INTERPOLATE_NEAREST`) and bilinear filtering
(`PDC_GL_INTERPOLATE_BILINEAR`) when the frame is scaled larger than its native
size. Bilinear filtering is slightly slower and appears soft, while
nearest-neighbor is faster and "pixelated".

Integration with SDL
--------------------

Unlike the SDL port, you shouldn't muck around too much with SDL functions.
SDL_Surfaces can't be used for rendering here. But there's a couple of things
you can still safely do.

```
PDCEX SDL_Window *pdc_window;
```

You can programmatically change the size of the window with
`SDL_SetWindowSize()` and `SDL_SetWindowFullscreen()`.

Interaction with stdio
----------------------

As with X11, it's a bad idea to mix curses and stdio calls. (In fact,
that's true for PDCurses on any platform; but especially these two,
which don't run under terminals.) Depending on how SDL is built, stdout
and stderr may be redirected to files.


Distribution Status
-------------------

The files in this directory are released to the public domain.


Acknowledgements
----------------

Original SDL port was provided by William McBrine
SDL2 modifications by Laura Michaels and Robin Gustafsson
OpenGL rendering by Julius Ikkala
