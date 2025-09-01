PDCursesMod for DRM and Linux framebuffer
=========================================

This directory contains source code to support PDCursesMod using either the 'traditional',  now-deprecated [Linux framebuffer](https://en.wikipedia.org/wiki/Linux_framebuffer) library,  or the newer [Direct Rendering Manager (DRM)](https://en.wikipedia.org/wiki/Direct_Rendering_Manager) library,  which works on Linux and *BSD.  Much of the [VT port](../vt) is recycled;  for example, the input routines are identical.  You will see that some source files simply `#include` a file from the VT port.

Shortcomings (which should be addressable) are :

- It assumes 8 or 32 bits/pixel.  It should be relatively easy to support other bit depths with suitable changes to the `PDC_transform_line()` function in `pdcdisp.c`,  but I don't currently have a system that supports 15-, 16-, or 24-bits per pixel.   (Fortunately,  such displays appear to be getting rare.  I had to dig out an elderly laptop to test the 8-bit display.)
- Italic and bold fonts are synthesized from the given font,  but it would be relatively easy to let specific fonts be used for that purpose.

The default font for 8-bit-character builds,  borrowed from [DOSVGA](../dosvga),  is fixed at 8x14.  For wide-character builds,  the default is an 8x16-pixel subset of Unifont.  Set the environment variable `PDC_FONT` to point to the name of a PSF1, PSF2,  or VGA font to use that font instead.  (See `psf.c` for comments on these font formats.)  Hit Alt-Minus to toggle between the built-in and the `PDC_FONT`-specified fonts.  Add more fonts with `PDC_FONT2`,  `PDC_FONT3`,  etc;  Alt-Minus will then cycle among all specified fonts and the built-in one.

Alt-/ (Alt-Slash) will rotate the screen 90 degrees clockwise;  repeat for 180- and 270-degree rotation.  This may be useful on phones and other handheld,  hand-rotatable displays.  It also can help if you have a monitor set up in portrait mode.

You also/in addition can export the environment variable `PDC_ORIENT=1` to get a 90-degree clockwise rotation,  `PDC_ORIENT=2` for 180,  `PDC_ORIENT=3` for 270.

With the Linux framebuffer,  all monitors will be used.  (I've been unable to find a way around this.)  With DRM,  the default one will normally be used.  But you can set `PDC_SCREEN=1`, `PDC_SCREEN=2`, `PDC_SCREEN=3`,  etc. as an environment variable to specify a particular display.

Also with DRM,  you can cycle among available displays with Alt-=.  On my system,  at least,  this only works when one is root.  I've yet to figure out why.

Fullwidth characters can be displayed,  using an extension to the PSF2 font in which they are split into 'left half' and 'right half' glyphs.  See `pdcdisp.c` and/or the comments in `hex2psf2.c` (in the `junk` repository) for details.

Possible 'to do' items
----------------------

In no order :

- Fix the above shortcomings.
- Fallback fonts.
- Programmatic resizing through `resize_term()`,  both before and after calling `initscr()`.
- User-resizing and moving of windows.
- Reserve a line at the top wherein the application title and close/full-screen/minimize buttons can go.  The `ripoffline()` function may be useful here.

Building
--------

Run `make`, `make WIDE=Y`, or `make UTF8=Y`. Add `DLL=Y` to get a shared library (.so) on *nix builds.  On Linux,  add `DRM=Y` to get a DRM version;  otherwise,  you'll get a Linux framebuffer version.  On *BSD,  use `gmake`,  and you get DRM no matter what (as the name implies,  the Linux framebuffer is Linux-only).  Add `HAVE_MOUSE=Y` for mouse support (this uses the `evdev` library).  Run `make install` (you'll probably need to be root or use `sudo` for this) to install the shared library.

Permissions
-----------

Access to either the framebuffer or to DRM requires that a user be in the `video` group.  Access to the mouse requires a user to be in the `input` group.

Caveats
-------

See above.  This all works nicely and stably,  but it lacks support for 15 and (possibly) 16-bit and 24-bit displays.  (Such support should be easy to add... if such displays could be found.)

Distribution Status
-------------------

The files in this directory are released to the Public Domain.
