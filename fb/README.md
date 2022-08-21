PDCurses for Framebuffer
========================

This directory contains source code to support PDCurses using either the 'traditional',  now-deprecated [Linux framebuffer](https://en.wikipedia.org/wiki/Linux_framebuffer) library,  or the newer [Direct Rendering Manager (DRM)](https://en.wikipedia.org/wiki/Direct_Rendering_Manager) library,  which works on Linux and *BSD.  Much of the [VT port](../vt) is recycled;  for example, the input routines are identical.  You will see that some source files simply `#include` a file from the VT port.

Shortcomings (which should be addressable) are :

- It assumes 8 or 32 bits/pixel.  It should be relatively easy to support other bit depths with suitable changes to the `PDC_transform_line()` function in `pdcdisp.c`,  but I don't currently have a system that supports 15-, 16-, or 24-bits per pixel.   (Fortunately,  such displays appear to be getting rare.  I had to dig out an elderly laptop to test the 8-bit display.)
- The mouse is not supported.  It looks as if the `uinput` system allows one to access the mouse without needing X;  I need to investigate.  `gpm` may be a better choice.
- Italic and bold fonts are synthesized from the given font,  but it would be relatively easy to let specific fonts be used for that purpose.

The default font,  borrowed from [DOSVGA](../dosvga),  is fixed at 8x14.  Set the environment variable `PDC_FONT` to point to the name of a PSF1, PSF2,  or VGA font to use that font instead.  (See `psf.c` for comments on these font formats.)  Hit Alt-Minus to toggle between the built-in and the PDC_FONT-specified fonts.  Add more fonts with PDC_FONT2,  PDC_FONT3,  etc;  Alt-Minus will then cycle among all specified fonts and the built-in one.

Possible 'to do' items
----------------------

In no order :

- Fix the above shortcomings.
- Display on multiple monitors.  (Doesn't look feasible with the Linux framebuffer version,  I think,  but would be relatively easy on DRM.)
- Fallback fonts.
- Fullwidth characters.  For these,  we may need one font that is,  say,  8 pixels wide (for most characters) and one that is 16 pixels wide for the fullwidth characters.  That may be an extension of the above task of having fallback fonts : "Didn't find the desired glyph in the 'normal' font?  Maybe it's in a fallback font."
- Allow for rotated displays.  (As with the aforementioned font switching,  this results in a screen resizing,  so it'll only work with Curses programs that handle resizes.  On phones,  the resizing may occur when you rotate the phone.)
- Programmatic resizing through `resize_term()`,  both before and after calling `initscr()`.
- User-resizing and moving of windows,  if we can get the mouse to work.
- Reserve a line at the top wherein the application title and close/full-screen/minimize buttons can go.

Building
--------

Run `make`, `make WIDE=Y`, or `make UTF8=Y`. Add `DLL=Y` to get a shared library (.so) on *nix builds.  On Linux,  add `DRM=Y` to get a DRM version;  otherwise,  you'll get a Linux framebuffer version.  On *BSD,  use `gmake`,  and you get DRM no matter what (as the name implies,  the Linux framebuffer is Linux-only).  Run `make install` (you'll probably need to be root or use `sudo` for this) to install the shared library.

Caveats
-------

See above.  This all works nicely and stably,  but it lacks mouse input and support for 15 and (possibly) 16-bit and 24-bit displays.

Distribution Status
-------------------

The files in this directory are released to the Public Domain.
