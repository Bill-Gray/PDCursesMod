PSF2 fonts
==========

At present,  these fonts work only with the Linux framebuffer port.  It is likely that they'll eventually be used with SDLn and DOSVGA,  and possibly others,  thereby enabling a lot of redundant code to be consolidated.

SDLn in non-wide mode uses its own font format,  in which the font is a monochrome Microsoft(R) Windows .bmp file,  with 32 characters across and eight down.  For example,  this is the default SDLn font :

![image](https://www.projectpluto.com/temp/pdc_default_font.bmp)

At one time,  a [wide selection of SDL fonts](https://sourceforge.net/projects/pdcurses/files/sdlfonts/1.0/sdlfonts.zip/download) was distributed with PDCurses.  The `bmp2psf2` program was written to convert SDL-formatted fonts into the [`psf2` format](https://www.win.tue.nl/~aeb/linux/kbd/font-formats-1.html).  `psf2` is more widely used and allows for more than 256 glyphs (in fact,  up to 2^32-1 glyphs,  with Unicode information.)

This directory contains the SDL fonts after conversion to the `psf2` format.  See the [README file](README) for details on which fonts are which,  history,  and copyright/licence info.

[Tools are available elsewhere for manipulating/converting `psf2` fonts.](https://www.seasip.info/Unix/PSF/)
