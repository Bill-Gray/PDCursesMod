PDCurses for Linux framebuffer
==============================

This directory contains source code to support PDCurses using the Linux framebuffer library.  Much of the VT port is recycled;  for example, the input routines are identical.  You will see that some source files simply #include the version from the VT port.

This is in a primitive,  but largely working,  state. Shortcomings (which should be addressable) are :

- It assumes 8 or 32 bits/pixel.  It should be relatively easy to support other bit depths with suitable changes to the `PDC_transform_line()` function in `pdcdisp.c`,  but I don't currently have a system that supports 15-, 16-, or 24-bits per pixel.
- The mouse is not supported.  It looks as if the `uinput` system allows one to access the mouse without needing X;  I need to investigate.
- Italic and bold fonts are synthesized from the given font,  but it would be relatively easy to let specific fonts be used for that purpose.

The default font,  borrowed from DOSVGA,  is fixed at 8x14.  Set the environment variable `PDC_FONT` to point to the name of a PSF1, PSF2,  or VGA font to use that font instead.  (See `psf.c` for comments on these font formats.)

Possible 'to do' items
----------------------

In no order :

- Fix the above shortcomings.
- Possibly display on multiple monitors,  and allow for rotated monitors.
- Fallback fonts.
- Fullwidth characters.  For these,  we may need one font that is,  say,  8 pixels wide (for most characters) and one that is 16 pixels wide for the fullwidth characters.  That may be an extension of the above task of having fallback fonts : "Didn't find the desired glyph in the 'normal' font?  Maybe it's in a fallback font."
- Combining characters.  This may actually not be too difficult,  if ORring glyphs together works,  as it theoretically ought.
- Resizing by selecting a different font.  You'd probably specify some fonts with `PDC_FONT2`, `PDC_FONT3`,  etc.,  and could then cycle through them with Ctrl-Shift-Right Arrow and Ctrl-Shift-Left Arrow.  On phones,  resizing may also occur when you rotate from landscape to portrait.
- Programmatic resizing through `resize_term()`,  both before and after calling `initscr()`.
- User-resizing and moving of windows,  if we can get the mouse to work.
- Reserve a line at the top wherein the application title and close/full-screen/minimize buttons can go.

Building
--------

Run `make`, `make WIDE=Y`, or `make UTF8=Y`. Add `DLL=Y` to get a shared library (.so) on *nix builds.  Run `make install` (you'll probably need to be root for this) to install the shared library.

Caveats
-------

See above.  This is still a little rough around the edges.

Distribution Status
-------------------

The files in this directory are released to the Public Domain.

