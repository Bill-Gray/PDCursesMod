PDCurses for OpenGL 4.3
=======================

This is a port of PDCurses for OpenGL 4.3 on top of SDL2. It's based on the
SDL2 port, but uses OpenGL for character drawing.


Building
--------

- Currently, this port requires using the included CMakeLists.txt for building.
- Depends on SDL2 and SDL2_ttf


Usage
-----

There are no special requirements to use PDCurses for OpenGL -- all
PDCurses-compatible code should work fine.

Unlike the SDL2 PDCurses, this port assumes that the user isn't doing any SDL
or OpenGL stuff behind it's back, as that would break our assumptions of the
OpenGL state machine. However, you can change the SDL window parameters safely
(size, fullscreen).

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


Interaction with stdio
----------------------

As with X11, it's a bad idea to mix curses and stdio calls. (In fact,
that's true for PDCurses on any platform; but especially these two,
which don't run under terminals.) Depending on how SDL is built, stdout
and stderr may be redirected to files.


Distribution Status
-------------------

The files in this directory are released to the public domain, EXCEPT for the
'glad'-folder (https://github.com/Dav1dde/glad).


Acknowledgements
----------------

Original SDL port was provided by William McBrine
SDL2 modifications by Laura Michaels and Robin Gustafsson
OpenGL rendering by Julius Ikkala
