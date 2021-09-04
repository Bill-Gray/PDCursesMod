PDCurses for Linux framebuffer
==============================

This directory contains source code to support PDCurses using the Linux framebuffer library.  Much of the VT port is recycled;  for example, the input routines are identical.  You will see that some source files simply #include the version from the VT port.

This is in a primitive,  but largely working,  state. Shortcomings (which should be addressable) are :

- It assumes 32 bits/pixel.  It should be relatively easy to support other bit depths with suitable changes to the `PDC_transform_line()` function in `pdcdisp.c`.
- The mouse is not supported,  at least not on my machine.  (I've never gotten the mouse to work in the Linux console.  Almost certainly something I'm doing wrong.)
- Italic and bold fonts are synthesized from the given font,  but it would be relatively easy to let specific fonts be used for that purpose.

The default font,  borrowed from DOSVGA,  is fixed at 8x14.  Set the environment variable PSF_FONT to point to the name of a PSF1, PSF2,  or VGA font to use that font instead.  (See `psf.c` for comments on these font formats.)

Possible 'to do' items
----------------------

In no order :

- Fix the above shortcomings.
- Possibly display on multiple monitors,  and allow for rotated monitors.
- Fallback fonts.
- Fullwidth characters.  For these,  we may need one font that is,  say,  8 pixels wide (for most characters) and one that is 16 pixels wide for the fullwidth characters.
- Combining characters.  This may actually not be too difficult,  if ORring glyphs together works,  as it theoretically ought.
- Resizing by selecting a different font.  You'd probably have to specify some fonts,  and could then switch among them with Ctrl-+ and Ctrl--.
- Programmatic resizing through `resize_term()`,  both before and after calling `initscr()`.
- User-resizing and moving of windows,  if we can get the mouse to work.
- Reserve a line at the top wherein the application title and close/full-screen/minimize buttons can go.

Building
--------

Run `make` or `make WIDE=Y`. Add `DLL=Y` to get a shared library (.so) on *nix builds.  Run `make install` (you'll probably need to be root for this) to install the shared library.

Caveats
-------

See above.  This is still a little rough around the edges.

Distribution Status
-------------------

The files in this directory are released to the Public Domain.

