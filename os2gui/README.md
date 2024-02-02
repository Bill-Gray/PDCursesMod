PDCurses for OS2GUI
===================

This directory contains PDCurses source code files specific to OS/2
graphics mode.

Building
--------

- Choose the appropriate makefile for your compiler:

        Makefile     - EMX 0.9d+
        Makefile.wcc - Open Watcom 1.9+

- Optionally, you can build in a different directory than the platform
  directory by setting PDCURSES_SRCDIR to point to the directory where
  you unpacked PDCurses, and changing to your target directory:

        set PDCURSES_SRCDIR=c:\pdcurses

- Build it:

        make -f makefilename

  (For Watcom, use "wmake" instead of "make"; for MSVC, "nmake".) You'll
  get the libraries (pdcurses.lib or .a, depending on your compiler; and
  panel.lib or .a), the demos (*.exe), and a lot of object files. Note
  that the panel library is just a copy of the main library, provided
  for convenience; both panel and curses functions are in the main
  library.

  You can also give the optional parameter "WIDE=Y", to build the
  library with wide-character (Unicode) support:

        wmake -f Makefile.wcc WIDE=Y

  Another option, "UTF8=Y", makes PDCurses ignore the system locale, and
  treat all narrow-character strings as UTF-8. This option has no effect
  unless WIDE=Y is also set. This was originally provided to get around
  poor support for UTF-8 in the Win32 console:

        wmake -f Makefile.wcc WIDE=Y UTF8=Y

  OS2GUI doesn't have the same limitations as the OS/2 console flavor,
  but UTF-8 and non-UTF-8 versions are still available.  If nothing else,
  this means that if you've built a OS/2 console PDCurses DLL with any
  configuration,  you can build a matching OS2GUI DLL and swap between
  console or GUI PDCurses just by swapping DLLs.

  You can also use the optional parameter "DLL=Y" with Visual C++,
  MinGW or Cygwin, to build the library as a DLL:

        nmake -f Makefile.vc WIDE=Y DLL=Y

  When you build the library as a Windows DLL, you must always define
  PDC_DLL_BUILD when linking against it. (Or, if you only want to use
  the DLL, you could add this definition to your curses.h.)

Distribution Status
-------------------

The files in this directory are released to the Public Domain.

Acknowledgements
----------------

Based heavily on the Win32 console flavor of PDCurses by Chris Szurgot
<szurgot[at]itribe.net>,  ported to Win32 GUI by Bill Gray
<pluto[at]projectpluto.com>.
