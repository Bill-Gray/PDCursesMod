PDCurses for Linux framebuffer
==============================

This directory contains source code to support PDCurses using the Linux framebuffer library.  Much of the VT port is recycled;  for example, the input routines are identical.  You will see that some source files simply #include the version from the VT port.

This is in a primitive,  but largely working,  state. Shortcomings (which should be addressable) are :

- It assumes 32 bits/pixel.  It should be relatively easy to support other bit depths with suitable changes to the `PDC_transform_line()` function in `pdcdisp.c`.
- The mouse is not supported,  at least not on my machine.  (I've never gotten the mouse to work in the Linux console.  Almost certainly something I'm doing wrong.)
- The font,  borrowed from DOSVGA,  is fixed at 8x14.  This should be fixed (without much trouble) using the PSF1/PSF2/vgafont code in the Bill-Gray/junk repository;  see `psf.c` and `psf_test.c`.  We should then be able to run with pretty much any font you want.
- Italic and bold fonts are synthesized from the DOSVGA font,  but it would be relatively easy to let specific fonts be used for that purpose.
- Wide character display doesn't currently work.  Use of PSF,  allowing Unicode,  should fix that (and I'll then embed a suitable PSF font instead of the vgafont one.)  I've some thoughts for handling fullwidth characters;  combining characters will be tougher.

Building
--------

Run `make` or `make WIDE=Y`. Add `DLL=Y` to get a shared library (.so) on *nix builds.  Run `make install` (you'll probably need to be root for this) to install the shared library.

Caveats
-------

See above.  This didn't exist two days ago;  it's rough around the edges.

Distribution Status
-------------------

The files in this directory are released to the Public Domain.

