Generally speaking,  this history mentions only the more significant
changes.  See the git log for full details.

Current version - 2025 July 15
==============================

Minor new features
------------------

- More fixes a la those in 'upstream' PDCurses to make MANUAL.md more
  markdown-friendly.  3e528a85d4  ed3103d705

- Framebuffer and DRM ports now have full mouse support.  7ec7b956f0
  2728d46838  04a5f1de93

- Zooming/panning around in 'picsview' no longer flickers.  af4a7c5d72

- Start made on making use of individual flags in curses_trace().
  Previously,  debug output was all or nothing (still is,  mostly,  but
  that can now be changed).  8f73eb0c1b

Bug fixes
---------

- C23-compliant compilers (and some others) objected to the casting of
  function pointers returned by GetProcAddress().  ce60aa6608

- If PDC_NCMOUSE is #defined,  the behavior of the mouse routines should
  copy that of ncurses,  warts and all.  Scroll-up and down are mapped
  to presses on buttons 4 and 5,  and tilt-scroll events and "real"
  button 4 and 5 events are ignored.  0de9f723e3

- Minor fixes for Digital Mars and OpenWATCOM compilation of WinCon and
  WinGUI.  a4c01532e8  6af988940d  6eedea061c

PDCursesMod 4.5.2 - 2025 June 19
================================

Minor new features
------------------

- Framebuffer/DRM shows a mouse cursor now,  if built with HAVE_MOUSE=Y.
  It doesn't actually respond to mouse events yet,  but that should be
  relatively straightforward.  7578a604b9

- x11new and framebuffer/DRM ports cleaned up,  READMEs updated,  some
  unnecessary code removed.  b08805c7dd  6221595646  f300bfa0c0  bae06a863c

- Fixes for C90 compliance : removed some C++-style // comments,  moved
  variables to the top of code blocks,  used TRUE/FALSE instead of
  true/false.  880000943a  9efd32336c

- In x11new,  you can hit Ctrl-Minus and Ctrl-Plus to go to the next smaller
  or larger font size,  respectively.  43066a8c8c

- The `PDC_KEY_MODIFIER_REPEAT` modifier was not applied in SDL1 and x11new,
  and was broken in SDL2.  83fa89ce74  6bf92e1a06

- Changed some documentation to be more markdown-friendly when read into
  MANUAL.md,  a la William McBrine's work upstream.  07a5469785
  b786e76322  1cc01e4ec3  7cc0f0d1bb  c58fd922d0  a06e0be5ab

Bug fixes
---------

- getch() triggered an assert() if fed a character that expanded to a
  multi-byte string in PDC_FORCE_UTF8 builds (issue #335,  reported by
  Robin Haberkorn).  6306ac2d48

- WinCon/pdckbd.c contained a poorly-written cast to which the C23 standard
  objects,  resulting in warnings/errors when built to that standard
  (issue #333).  Fix supplied by Gisle Vanem.  2426dbc85e

- The `PDC_KEY_MODIFIER_REPEAT` modifier was broken in SDL2.  6bf92e1a06

PDCursesMod 4.5.1 - 2025 May 12
===============================

Major new features
------------------

- Added an 'x11new' port : X11 written from scratch to require fewer
  dependencies and to allow a memory-leakage-free build.

Minor new features
------------------

- Framebuffer/DRM : simplified specifying which display to use.  If you
  have suitable permissions,  Alt-= will cycle through displays.
  db3cbb7b0e

- `testcurs` now allows one to specify mouse mask parameters more precisely,
  and/or to set a zero mouse interval (which should result in clicks being
  converted to presses and releases).  Also,  can test using the ncurses
  mouse interface instead of the 'classic' undocumented SysV mouse interface.
  de6c742b4a   7a520f6261   91645ceb9b

- VT port now works on Haiku.   e8f098d6de

Bug fixes
---------

- Framebuffer/DRM port cursor didn't rotate when you rotated the screen.
  f3188affb9

- When restarting Curses,  the foreground/background weren't stored,
  resulting in odd colors.  433dbfb1d7

- The ncurses forms library does not build correctly for wide characters.
  Until it's fixed,  it and the tests relying on it are only built in
  narrow-character (8-bit) mode.     0f3125393c

- Fixed possible read-outside-bounds error in UTF8 conversion.  5a6170982e

- `testcurs` called scanw() with a "plain" %s format specifier.  The lack of
  a width left it open to buffer overflow if more than 79 bytes were entered.
  8bf383a7b5

- VT build failed when using GNU make to cross-compile demos and tests for
  Windows using MinGW.  d391921be0

- Mouse input inconsistently respected `mousemask()` on most platforms.
  The common logic for combining press/releases into clicks,  double- and
  triple-clicks has been abstracted into `common/mouse.c`,  and is now used
  by the WinCon,  WinGUI,  x11new,  and VT platforms (and will probably be
  used eventually by most other platforms).  See issue #330.   52d206870c
  9e857e7792  812bd6953a  592d27f164

PDCursesMod 4.5.0 - 2024 Dec 31
=================================

Major new features
------------------

- Added an OS/2 GUI port,  provided by chasonr.  23b1fbed61  267f92a2c0

- The ncurses 'menu' library can be compiled and used with PDCursesMod,
  and the ncurses program 'demo_menus' built to test it out.  (The
  'forms' library can be built in 8-bit character mode -- it currently
  uses ncurses internals for wide mode -- but doesn't appear to
  actually work yet.)  07bc4ac785  1ad9562b9a

Minor new features
------------------

- The DRM and Linux framebuffer port now allows for screen rotation.
  Hit Alt-/ for a 90-degree rotation.  This should pave the way for
  support on phones and other rotatable displays (and already helps if
  you have a monitor in portrait mode).  0e41ba5e68  1b7b432f9c

- Added is_cbreak(), is_echo(), is_nl(), is_raw() functions to access
  members of the non-opaque SCREEN struct.  The first two replace
  PDC_getcbreak() and PDC_getecho().  98b6969a7e

- Windows can be created,  moved,  and resized that go off the right
  and bottom edges of the screen,  a la ncurses.  (I think this was
  an acknowledgment of the fact that after resizing a screen,  windows
  might go over those edges anyway.)  Windows still cannot go off the
  left or top edges.   bc51d18ff1

- Added a new 'test' program for the issues mentioned above with
  scrolling and winsertln(), wdeleteln(), winsdelln().  98c7405a3f

- Three new small test programs to investigate various aspects of
  how curses works (or doesn't work).  5cef1e3d00

- The 'tests' programs can now be built,  on some platforms,  with
  'make -f Makefile tests'.  200b548516  e611673b49  63138eaf34
  80fdfe7095  218aa8c0ed

- Switched from defaulting to ncurses-6.4 tests to ncurses-6.5.

- WinGUI now defaults to resizable windows,  similar to SDLn,  WinCon,
  and X11.  58807ea498

- winsnstr() handled 513 _bytes_ from the input,  instead of the clearly
  intended 513 wchar_ts.  This worked,  but was an unnecessarily low
  limitation.  It is now fixed,  and the function can now handle
  arbitrarily long strings via recursion.  Added some code to test this
  in 'show_col.c'.  3f8dfa9e06   18ef78de69

- PDC_wcwidth( ) updated from Unicode 14.0.0 to 16.0.0   93e32ef a5f13c2

Bug fixes
---------

- In rare cases,  blinking on the framebuffer port could be disabled
  due to an integer rollover.  eaee2a61f2

- In WinGUI,  we could potentially read memory out of bounds.  From
  slipher.  70d27245d1

- In WinGUI,  theoretically 'monospaced' fonts were not necessarily
  monospaced.  Reverted to drawing each character separately.  b99fed1acb

- In WinGUI,  when redrawing a string of characters,  the preceding and
  subsequent glyphs were also redrawn.  This was apparently needed at
  one time to avoid stray pixels,  but appears to be unnecessary now.
  8466f73e21

- In WinGUI and WinCon,  PDC_clearclipboard() would fail if the clipboard
  was not open.  From Mark Hessling.  a071130c95

- If napms() was called with a negative value,  it would lock. 65dfa7e61a

- Framebuffer/DRM port didn't redraw upon changing fonts if the new font
  was the same size as the old one.  c714cd9bcc

- wsetscrreg() required the cursor for the window to be positioned
  between the 'top' and 'bottom' of the new scroll region.  5a0d5becee

- wdeleteln() would delete the line currently under the cursor,  then
  bring up lines down to the end of the current scroll region,  inserting
  a blank line there.  It really should delete all the way to the bottom
  of the window.  See also wmcbrine/PDCurses/issues/165.  b02f94fd3d

- The 'picsview' demo had a possible buffer overrun error.  Reported and
  fix proposed by Michael Knap.  fcd2dd0cb8

- Plan9 port ran into a compile error due to different timekeeping
  functions.  Also was missing a few demos and term.h.  1e8472cdbf

- Removed an assert() in mvwin() when the window was moved outside the
  screen area.  Some programs (including 'test_pan') assume they can try
  moving a window,  then check the return value to see if it was actually
  on screen.  c801354044

- PDC_set_box_type() has been removed,  because it's an unneeded redundancy.
  You can get double-lined or thick-lined boxes,  and/or rounded corners,
  via the standard Curses wborder_set() function,  and it'll work with
  ncurses and other libraries.  'test_pan' revised to provide an example
  of how that's done.   f8d87ed549  099fa2876c

- printw() overflowed a buffer when asked to output more than 512 bytes.
  ddf80d010f

- napms() rounded _up_ to the nearest clock tick on DOS and DOSVGA;  it now
  rounds _to_ the nearest clock tick (with at least one tick being used).
  Modified the documentation to reflect this.  785113f0b9

- PDC_wcstombs() could overflow a buffer in the UTF8 decoding.  7d704dd618

- DOS and DOSVGA builds failed on Digital Mars.  Several small fixes were
  required.   981893b997  d22725b90a  32f22660c4  fd1ddf866c

- Fixes for Borland Turbo C compilation. a59f452e78  26128c29aa
  d6b7e998eb

- panel_hidden() is now portable, returning TRUE (1) and FALSE (0) instead
  of OK (0) and ERR (-1) This may break existing applications, so you possibly
  want to check for the PDCursesMod build (PDC_BUILD < 4500).

PDCursesMod 4.4.0 - 2023 November 30
===================================

   Note that because of SCREEN and WINDOW now being opaque,  the WA_
flags and some A_ flags changing,  and KEY_MAX being redefined, this
is _not_ binary compatible with 4.3.x.  However,  you'll get a link
error if you get versions mixed up,  so this shouldn't cause any
real trouble.

Major new features
------------------

- The SCREEN and WINDOW structures are now opaque.  11671880e2
  3259719b37

- The WA_ flags are now distinct,  in builds with 64-bit chtypes and
  attr_ts (the default).  This led to considerable documentation
  changes and an attempt to make the distinction between WA_ macros
  (for use with wattr_t) and A_ macros (for use with chtype) clearer.
  Most implementations of curses have wattr_t == chtype,  including
  PDCurses* and ncurses,  but you can't really rely on it.
  78658c94cd

- KEY_MAX was increased to reserve some key codes for future use.
  16ac494c25

- The FILE arguments passed to newterm() are now actually used on the
  VT platform.  Changes by Pavel Stehule.  9c16db9589

Minor new features
------------------

- SDL1,  SDL2,  and GL support double and triple mouse clicks.
  From Lyle Tafoya.  a4e563d763

- 'widetest' now tests copying of both precomposed and combining
  characters with getcchar() and setcchar().  19169a1b18

- VT flavor gets the correct initial screen size in Windows.
  d62083a096

- 'panels' library almost completely rewritten.  The new version is
  better at figuring out what parts need to be updated,  and is
  generally easier to understand.  8830b71392.

- PDC_transform_line_sliced() used to break up lines to ensure that
  PDC_transform_line() will not be fed packets of excessive size,  nor
  packets that continue after fullwidth or combining characters.
  This avoids buffer overruns and problems with mishandled characters.
  cb8c87b287

- 'testcurs' attribute test now lists which flags in term_attrs() are
  set.  4da423c59e

- Added some small test programs for very specific feature/bug tests.
  e6a5edee79

- You can set a 'thick' style for boxes/borders (wide-character builds
  only).  5fd12f8ec9

Bug fixes
---------

- WinGUI keyboard responsiveness could be poor if it wasn't getting
  enough CPU share.  087c20cc87

- Fixed a (very small) memory leak from reset_color_pairs.  9e236abb17

- Fixed some ncurses demo failures,  due to incomplete HAVE_xxx
  descriptions.  From William McBrine's PDCurses.  2d114bf406

- Panels were only redrawn on window resizes if they overlapped other
  panels,  and even then,  only the overlapping lines were redrawn.
  e2b2205da6

- Numeric keypad presses were duplicated on SDL2 and GL for Windows.
  7c26ec92ed

- The SDL2 makefile had a recursive definition of CC and PREFIX.
  ac96c5511e

- If the screen was resized,  either programmatically or by the user,
  the "pseudo-panel" for that screen was not resized.  6b571be281

- waddnwstr() could read characters one element past the array bounds.
  (Theoretically fixed in commit c03e650a70,  but I'd messed it up.)
  5735a1cb31

- touchoverlap() un-touched some parts of the window that didn't
  overlap.  5c0bc334f8

- Line attributes were not reliably updated after PDC_set_line_color().
  3b54290f13

- Ripped-off lines did not necessarily work initially and always got
  mangled when the screen was resized.  05e09e5368

PDCursesMod 4.3.7 - 2023 June 17
================================

Major new features
------------------

- SDL2 GL port,  from Julius Ikkala.  This is (on most systems)
  faster than the "traditional" SDL2 port.  Many commits,  starting
  at 4678131a39.

Minor new features
------------------

- Support on Windows for Unicode input in the SMP (codepoints above
  above 0xFFFF, for instance emojis.) 2050706649  75b62ea015

- WinCon: supports full unicode output including SMP.  3ec63c9067
  7089b2eae1  fac87bd938

- WinCon, WinGUI can be cross-compiled for Windows on Arm.  158e8ae5b7

- 'newtest': extended the SLK testing.  'newtest' and 'testcurs' now
  use wget_wch( ) in wide builds,  so the actual character is shown on
  input tests (instead of the multibyte stream).  'testcurs' gradient
  example (showing colors beyond 256 palette entries) is a little more
  logically set out.  c744e3ec03  7089b57d07

- WinCon can support a 'default' (transparent) background.  81cbefbd4b

- 'curspriv.h' modified to allow C++ compilation.  52a2fdf518

- Clarified compatibility for ACS_ and WACS_ box characters,  and added
  many missing aliases.  From a suggestion by Simon Sobisch.  e8f7b33bbf

- Fixed warnings with OpenWATCOM.  We now compile with -we ("treat
  warnings as errors").  f27369e93b  abbd8f5268  89e3426248

- If you try to compile the DOS version with WIDE=Y,  you get an error
  message explaining that wide characters don't work on that platform.
  d07b884502

- Now using ncurses-6.4 tests instead of 6.3.

Bug fixes
---------

- WinCon: Key modifiers were incorrectly cleared without a key event.
  9229f08604

- VT: Ctrl-Alt-letter input was off by one.  Hit,  say,  Ctrl-Alt-T,
  and you'd get Ctrl-Alt-U.  65568582af

- Fixed some casting issues with GetProcAddress( ) in Windows and a
  deprecation warning with MinGW,  with considerable help from Arnold
  Tremblay and Chuck Haatvedt.  Further work may be required (it works,
  but pedantic compilation can get you warnings).  bbae4bb0e8  28cbcfcc34

- CMake's list of demos is now up to date for DOS,  DOSVGA,  WinCon,
  WinGUI.  Others to follow.  4f3dc96436

- Pasting text from the clipboard was done with a buffer lacking room
  for a null terminator,  resulting in a memory overwrite.  Reported by
  nhmall (issue #295).  0a4e1e9065

- realloc( ptr, 0); was assumed to free ptr and return NULL.  This is
  usually true,  but isn't guaranteed and isn't true on FreeBSD,  and
  caused an assert to be triggered in endwin().  abbb6de7de

- 'terminfo' functions were not included in the DOSVGA builds,  nor in
  WinCon when using the Microsoft VC makefile.  Also,  the WinCon Digital
  Mars makefile was broken.  c51efbbf88

- 'picsview' and 'mbrot' demos now work correctly with ncurses.  Some
  unnecessary distinctions between PDCurses* and ncurses were removed.
  89727917e7

PDCursesMod 4.3.6 - 2023 April 12
=================================

Minor new features
------------------

- SDL2 can now show Unicode past 0xffff (Supplemental Multilingual
  Planes).  602bc380f81

- VT port supports output redirection.  a6618d94d7  6925d0f571
  65939aaccd

- WinGUI support DPI awareness,  for better font display on high-res
  screens.  012e3d90f8

- SDL1 and 2 and X11 support user handling of the window close button,
  using PDC_set_function_key.  89b2287d66  791318dfaa

- 'terminfo' functions (really stubs) are compiled and linked in for
  all platforms.  ab1c007273  248146021e

- Build support added for LLVM and Windows on ARM,  from Mioki.
  158e8ae5b7  2fb65608b3  86c1296d89  4884ce2c6d

Bug fixes
---------

- HiDPI support for WinGUI crashed (reported by Chuck Haatvedt),  fixed
  by Casey Langen.  489f5d6317

- Resizing 'ptest' caused it to move to the next panel display
  in the series.  0ed72b9f91

- Partial fix so that getch( ) will return byte codes,  expanding characters
  beyond 256 into multi-byte strings.  bd50b2b2ab

- Negative window sizes for newwin() and resize() were not error-checked,
  resulting in segfaults.  c6c7f2d074

- In Windows SDL2,  Alt-keystrokes were ignored (see wmcbrine/PDCurses#142).
  Fix provided by Benjamin Adamson.  7f1e1bba34

- In SDL2,  programmatic resizes caused further spurious resizes,
  leading to KEY_RESIZE events.  b09c91ea15

- Scrolling a window could result in a crash when the window was
  freed.   72521a1697

- Decoding UTF8 was broken for SMP.  ad7cd29a23

- WinGUI could get confused if the window was resized,  resulting in
  input being ignored.  bc340c0d20

- Text selection & copying could cause memory corruption with 4 byte UTF-8
  codepoints when WIDE=Y.  bff53ab114

See the git log for more details.

PDCursesMod 4.3.5 - 2022 November 27
====================================

Major new features
------------------

- WinGUI is again single-threaded,  resulting in considerably simpler
  code and fixing some resizing issues.  See pull request #240.
  6962ab6e9d

Minor new features
------------------

- Framebuffer port can now use either the (deprecated) Linux framebuffer
  system or the DRM (Direct Rendering Manager) system.  DRM is not
  supported on some older systems and the Linux framebuffer is not
  supported on newer ones.  DRM also can be used with *BSD;  the Linux
  framebuffer really is a Linux-only solution.  e9bc09c083

- GNU Makefiles for VT and framebuffer ports work out of the box on
  FreeBSD.  2b5f0036fe

- Much revision internal to the PDCursesMod library (no effect as far
  as users of the library are concerned) to move static variables into
  the SCREEN structure,  in a way that preserves binary compatibility
  and keeps those variables opaque outside the library.  0d5e4dc5b4
  3e167172ff  7449df5369

- delwin() now returns ERR if you attempt to delete a window with
  active subwindows,  or a window that wasn't allocated by PDCursesMod
  or which has already been freed.  c643c0da95  3b14813dbb

- delscreen() deletes all windows associated with a SCREEN.  26e473c60b

- Made the PANEL and PANELOBS structures opaque.  f55e55a0fb  78039d10c3

- Checking for key hits is much,  much faster on most platforms.
  6612df5af6

- Added ncurses extensions for opaque WINDOW structs,  from Markus Uhlin.
  7bb822b2a4  22fecc1cf6

- You can specify the library name during a GCC build with LIBNAME=(name)
  and the shared library name with DLLNAME=(name).  Support is,  as yet,
  incomplete,  but it should be possible to expand this to MSVC and
  OpenWATCOM builds.   44e5aa323e

- 'ptest' and 'speed' demos free all memory on exit.  This helps (slightly)
  when testing with Valgrind.

- On gcc and mingw builds for WinCon, WinGUI,  and VT,  one can run 'make
  configure (options)' to revise 'curses.h' to use the specified options.
  This may be extended to other compilers and platforms.  fe39e244d2
  9c0f475287

Bug fixes
---------

- In some situations,  cursor movement/changes were not immediately made
  on VT and SDL2.  a805ec2b15  18abe3dbd5

- Keyboard entry in SDL2 was defective for some numeric keypad hits,  and
  shift-F(n) keys gave no response.  d87b7bd917  8a41f2ed57

- PDC_free_memory_allocations() (see below) was unnecessary;  if we follow
  the specifications for delscreen(),  as we now do,  all that memory
  gets freed in the natural course of things.  0223039ec0  60138ca8ec

- SDL2 had problems finding the default TrueType font on Rocky Linux 9
  builds.  Mark Hessling added a bit of code so that a secondary default
  path is specified;  if one path fails,  we try the other.  1e4bcc9676

- SDL1 was similarly updated;  it also got some better default locations
  for TrueType fonts on Windows and Apple,  borrowed from fixes that had
  already been done for SDL2.  9cf041e835

- Both SDL1 and SDL2 failed to free some memory when delscreen(SP) was
  called.  The result was that you couldn't cleanly shut down curses
  completely and restart it.  c62edf7752

- Fixed some problems with Digital Mars compiles and added missing demos.
  e798d30e63  4781b742b5  e39528f9e8  5efbea3676  aba598f492

- wgetnstr() was broken on platforms where wint_t != wchar_t.  54bc1a16fb

- AltCharSet display was broken in DOSVGA 8-bit character builds. e00e33650b

- getch() and wget_wch() could return erroneous values in WinCon for values
  greater than 127.  e54d03fc80  e86d9c3568

- Made PDC_set_default_colors() an internal function.  Exposing it basically
  amounted to a bug.  f1786064ee acd143144f

- In SDLn in fullscreen mode,  mouse events near the bottom or right-hand
  sides of the screen could trigger segfaults.   03eb51cefc

PDCursesMod 4.3.4 - 2022 July 29
================================

Minor new features
------------------

- As a small step to avoiding direct access to the SCREEN structure
  outside private PDCurses code,  added PDC_getbreak() and PDC_getecho()
  functions to access SP->cbreak and SP->echo. 59bd4b5653  5da7c96fe8

- You can free remaining internal buffers by calling the new
  PDC_free_memory_allocations() function.  8d10534143  2259f29d5f
  (Note that this has been reverted.)

- On Linux,  we use the system-provided wcwidth() instead of our own.
  Aside from avoiding redundant code,  this may help in non-Unicode
  locales (our wcwidth() is for Unicode only).  fc520ca19f

- PDC_wcwidth( ) updated from Unicode 5.0 to 14.0.0,  and now uses the
  16-bit tables generated by uniset.  68b38102bc

- When building SDL1 on OS/2,  Ctrl-(letter) and Alt-(letter) combinations
  were not returned.  Pointed out and fix contributed by Mark Hessling.
  2968f3198d

- Mark also found some neglected VT key sequences and added them.
  c47ce9f724

- Windows clipboard functions consolidated in the 'common' directory.
  This provides the clipboard code for the WinCon and WinGUI platforms,
  and for VT when compiled in Windows.  It would probably work with SDL1
  when compiled in Windows,  but that would need testing.  02d55b890e

- putwin() should produce files that can be read by getwin() even if
  the putting code and the getting code are compiled with different
  chtypes (i.e.,  wide vs. narrow and 32-bit vs. 64-bit chtypes).
  6824e49bab  98e3f56580

- Miscellaneous code cleanup.

Bug fixes
---------

- CMake builds of SDL2 with WIDE=Y were not actually making wide builds.
  797f688dd4  99e90f20c5  f6b73892c3

- endwin() was not signal-safe.  While in endwin(),  we shouldn't free
  memory or have debug logging on (we were doing both).  See "upstream"
  issue wmcbrine/PDCurses#134 for discussion.  c5ca238fac  18ff2cb00c
  8d10534143

- WinGUI could block if there was no key delay (threading contention).
  b84cf0cbb8

- Alternative character set glyphs could be shown when they shouldn't
  be,  or not shown when they should be. d6d338a8ee

- Revised mouse click handling on VT.  The previous logic sometimes missed
  double-clicks and could get confused with press/move/release.
  93d0d931a0

- VT platform missed some function keys on some platforms.  043ca72355

- SDL2 got duplicated key presses if Shift-(numpad) keys were hit (issue #234).
  9b86c29aa1

- Fixed (I think) problem with AppVeyor failing to get OpenWATCOM 1.9 to
  work,  causing almost every run to fail. cb59c93039  61929c222d  18abcfdbd6

- wattr_on() and attr_on() ignored the 'opts' parameter.  When non-NULL,  it
  should (and now is) treated as a pointer to an integer color pair.
  23374b894b

PDCursesMod 4.3.3 - 2022 May 25
===============================

Major new features
------------------

- OS/2 code replaced with William McBrine's current version,  with some
  modifications to accommodate PDCursesMod.  This basically means that
  the OS/2 port works again (it was rather thoroughly broken).  0b3cbc038f
  bd39b611288

Minor new features
------------------

- Clipboard handling is implemented in a common manner for DOS,  DOSVGA,
  VT,  SDL1,  and framebuffer ports.  This means the VT and framebuffer
  ports actually store the clipboard text in a buffer and can retrieve it,
  instead of basically doing nothing at all.  050b3532e1 d97b6a4171

- Two new demos,  'mbrot' (Mandelbrot display,  exercises some of the
  more esoteric color/palette functions) and 'calendar' (shows a calendar).
  cbd59aca60

- Screen resizing no longer requires a call to resize_term(0,0).  No
  one appears to know why that call was required in the first place.
  Commit 3110e3f624.

- The 'opts' parameter for certain functions,  heretofore ignored,  can
  now be used to handle integer-sized color pair indices,  a la ncurses.
  9be6bde504

Bug fixes
---------

- Color 8 defaulted to black on X11, WinGUI, Linux framebuffer,  and
  the SDLn platforms.  It should be (and now is) a medium gray.
  ab8b038dc9

- Wide-character string input functions,  such as get_wstr(),  generated
  display artifacts when fullwidth text was input.  549e4635e6

- VT port handles Alt-letter + shift/control modifiers better,  setting
  the appropriate modifier flags.  74b84cc28f

- WinGUI now interprets Shift-numpad keys correctly.  eacd04d1aa

- VT and framebuffer ports (again) capture Ctrl-C with an interrupt
  handler.  Without this,  you could hit Ctrl-C and not have anything
  actually happen until getch() was called,  so your code could hang
  easily.  Ctrls-Z, S, Q are now enabled on both platforms.  cac097f235

- If the screen was user-resized,  KEY_RESIZE was only returned if you'd
  called `keypad( stdstr, 1)`.  That requirement appeared to be unneeded
  and has been removed (see issue #230 on GitHub).  3110e3f624

PDCursesMod 4.3.2 - 2022 February 06
====================================

Minor new features
------------------

-  In 32-bit chtype,  non-wide-character builds,  you get 4096 color
   pairs and all twelve attribute bits that you would get with any
   64-bit chtype build.  d498305c35

-  Color pair management for find_pair(), alloc_pair(), free_pair()
   sped up considerably.  4d41c85101  864f02523d7

-  'testcurs' modified to show A_DIM and to show supergradients when
   possible.  dfb2387ff8

-  Almost all ncurses demos are now built,  and this also now applies
   to X11.  d742c1c708  e11eb4ed11

-  Default framebuffer font now includes all WCS/WACS glyphs.  88fbc83b29
   41ab7af534

-  X11,  VT,  and framebuffer ports all compile with full warnings and
   -Werror.   eabeb4cb99  fa98024a54  3c77a5b569

-  New 'widetest' demo exercises getcchar() and setcchar(),  neither of
   which actually got tested at all in other demos.  This was actually
   added in 4.3.1,  though it wasn't built by default in most ports.
   0a815ea432  9dbd63846b  a75727c5b2  4cd9338b56

-  Automated builds on Windows now include the VT platform (commits
   ceeb2de9e5,  6c3791f447).  But VT codes are indifferently supported
   by Microsoft;  use of it is deprecated,   with WinCon or WinGUI being
   suggested instead (commit d6e517eb36).

-  The 'mangling' of initscr() (see curses.h) that was done to ensure that
   library version errors would be caught at link time is now done to
   'endwin()' instead.  We can really do either,  but it turns out that
   some build processes rely on initscr() being unmangled.  1e19a2eb96

-  PDC_set_function_key( ) is not really platform dependent.  This eliminated
   some redundant code and enabled Ctrl-C to be returned on WinGUI and WinCon.
   Added a corresponding PDC_get_function_key( ).  95b0843f69

Bug fixes
---------

-  VT and framebuffer ports returned 10 for Enter and Ctrl-M,
   instead of 13,  and ignored ^C and ^Z.  Fixed by Mark Hessling (issue #225
   and pull request #227).  0dace8c90e  ab0da09435

-  WinCon port could mangle attributes for non-ANSI characters.  It
   still can (unavoidably so),  but the damage no longer extends to
   adjacent ANSI characters.  Partly from a fix made by William
   McBrine to PDCurses.  6553e40a37

-  Removed completely meaningless PDC_MOUSE_POSITION and MOUSE_POS_REPORTS
   macros.  See issue wmcbrine/PDCurses#129.  b4e7429976

-  The terminfo dummy functions were not exported in shared library/DLL
   builds.  Fix by Simon Sobisch.  e8723349de  24ffa99b03

-  DOS alternative character set (ACS) was improperly displayed.  5db85b1157

PDCursesMod 4.3.1 - 2021 November 28
====================================

Major new features
------------------

-  Added a Linux framebuffer port.   dd5b99ce81 and many others.

Minor new features
------------------

-  Added the ncurses extension WACS_T_* (thick-line box character)
   alternate character set defines.     5cefe5fa00

-  Added four ncurses demos.  948f4cf41d

-  Eliminated CHTYPE_LONG and CHTYPE_64.  Neither is really needed
   anymore;  we just have CHTYPE_32.  e275cadd78  91b0b0dd5d

-  It should now be difficult,  if not impossible,  to link code for
   one PDCursesMod binary API to code for another.  7e06e8c264

-  Palette changes are redrawn immediately.  That was already true for
   DOS and 8-bit framebuffer ports,  where the palette has a hardware
   meaning;  it's now true in X11,  VT,  WinGUI,  etc.   e73cae9b2c

-  mmask_t can now be 64 bits (and by default,  it is).  This enables
   some unsnarling of mouse mask #defines and gets rid of the collision
   of triple-clicks with mouse move events,  and provides bits for
   future use.  908f1b3d32

Bug fixes
---------

-  Pads were incorrectly refreshed (issue wmcbrine/PDCurses#124).
   41da0b2fb8  9de6c83cd9

-  Odd input issue in WinCon (and presumably WinGUI);  see issue
   wmcbrine/PDCurses#126.  This may be revisited.   64d727571e

-  Background was improperly refreshed in pads.  See wmcbrine/PDCurses#122.
   b31f19f477

-  SDL2 window could be shown improperly initialized at startup.  9d30ffa6db

-  Ctrl-C stops the program in X11 and SDLn if noraw() has been called.  The
   framebuffer,  VT,  and WinGUI ports already did this.  210d49f48a

-  Surrogate pairs and combining characters are now handled correctly in the
   setcchar() and getcchar() functions.  3ef0592cfe  422bb2152e

-  X11 could block on getch() if nodelay() and PDC_set_blink() were both set
   to TRUE.  Fixes issue #216.  e20301ae19

-  Fixed assert errors in SDL2 at startup.  See issue #215.  5ee7cc0722

PDCursesMod 4.3 - 2021 August 25
========================================

Major new features
------------------

-  SDLs and X11 ports now support full colors,  a la WinGUI and VT.
   a266f923ee, b63b36f9b8, fee4af28c8, 8606b96111

Minor new features
------------------

-  ncurses tests build correctly again.  Moved to ncurses-6.2 tests,
   and they can now be built on VT,  WinCon,  and WinGUI (they already
   worked with X11 and SDLn).  f8dd18c0f2 4a71b26dd1 c8b67969d6
   edd97f38fd 68f1f1c299

-  Added ncurses extension functions find_pair(), alloc_pair(), free_pair()
   (see commit 19c14525e2) and reset_color_pairs() (534f24f547).

-  To test the above ncurses extensions,  added support to build
   'demo_new_pair' from the ncurses tests (commit d8951fadc9) and
   a new PDCursesMod-specific 'init_col' demo (commit 7b4d38a0ee).
   The latter also tests out handling of default colors and the
   ability to preserve the screen background... which appears to
   only work in the VT flavor,  and imperfectly there.

-  SDLs now have strikeout and overlined text.  91b280f442  So does X11
   5e5cd29617

-  SDLs now recognize some 'extended' media keys.  5e10ef17fc

-  SDLs and VT can be compiled as shared libraries and installed/uninstalled.
   b78cd8a33f bb8d408675 d0309c2963

-  VT has strikeout text in xterm,  some (not many) others.
   05af0c6810

-  Removed unnecessary 'extended' key definitions.  b9fafe60f1

-  Code compiles with -Wextra -pedantic on many flavors without warnings.
   674512453a, 6ad85268c5, 2648cdf5cd, others.

-  Some tests done using GitHub's CI.  3cb1b5f1bf,  several following
   commits.

-  New PDC_set_box_type( ) function to allow boxes/lines to be drawn using
   the doubled-line characters.  Modified 'test_pan.c' to demonstrate the
   new function.  ab7bad3981  dd981b0e78

-  WinGUI,  WinCon beep() is now in a separate thread,  so we don't
   hang the program while the beeping takes place.  a6212d94d2

-  WinCon can be compiled for 32-bit chtypes.

-  Added a PDC_set_default_colors( ) function.  Some platforms expect
   particular background/foreground colors by default.  This lets you set
   those colors.  (Note that this should have been,  and was made,  an
   internal function;  programs should use 'assume_default_colors()'.)
   b642c91279

-  In Windows,  one can use Ctrl-C/Ctrl-V to copy/paste,  instead of
   Ctrl-Shift-C/Ctrl-Shift-V.   9f487d4fea

-  Added assert()s on many NULL parameters.  Calling with such parameters
   is permitted,  but usually indicates bugs that should result in an assertion
   when in debugging mode.  96f4984f9f

-  In DOSVGA,  as in X11,  WinGUI,  and the SDLs,  you can specify an
   initial screen size using resize_screen() before actually starting PDCurses.
   0623c44ff0

-  _tracef(), trace(), curses_trace() added for ncurses compatibility.  This
   doesn't (as yet) actually add any capabilities;  it just lets you access
   existing capabilities in the same way you would if using ncurses.

Bug fixes
---------

-  WinCon and WinGUI key tables didn't go far enough and could crash if some
   unusual key codes were received.  See wmcbrine/PDCurses issue #118 (this
   is mostly a copy of wmcbrine's fix as suggested by zingchen).   fe124295be
   59aa2afce9

-  SDL2 display did not update after restoring from iconified form.
   2bb4284597

-  Both SDLs crashed if a background image was shown,  due to an out-of-bounds
   array access.  e19ea211fc fbf4ea1fec

-  DOS PDC_napms() failed for times greater than 18 minutes (would round
   time down to nearest multiple of 18 min).  acc1fa72fa

-  Buffer could overrun on extremely large (more than 512 column) displays
   in X11 (91b280f442), WinCon (51327cad4d)

-  Certain extended keys could crash WinCon, WinGUI.  d747d35ed8

-  Plugged some memory leaks in X11.  Much more to be done.  cb4344a163

-  In VT flavor,  the mix of printfs() and (unbuffered) write() calls
   wasn't interrupt-safe,  and led to some corruption in the output.  This
   is now fixed. 365b6cce79

-  Some cursor shapes in VT were mixed up.  1f1bbe7d8a

-  Fixes for SDL2 font colors when using non-mono bitmaps.  e2b2874e76
   d9306c09db

-  Computation of the font width in X11 was wrong,  leading to misaligned
   text.  This also mangled the code that checks that the bold and italic
   fonts are the same size as the 'normal' font.   54d721cafe

-  X11 mouse handling ignored mouse moves,  sometimes suppressed Ctrl/Alt/
   Shift modifiers,  and the logic was confusing;  other bugs may have
   lurked in there.   cfd7f64b95

-  Input in WinCon,  when compiled in narrow (8-bit,  non-PDC_WIDE) mode,
   could be mangled.  Shamelessly copied wmcbrine's fix from the commit
   in PDCurses e28e705d17438ffd (q.v.).   83dcf79672

PDCursesMod 4.2 - 2020 Oct 03
=============================

Major new features
------------------

-  Pulled in almost all of William McBrine's 3.8 and 3.9 changes.
   These include the single-process X11 port (much simpler and
   less prone to bugs),  common copy/paste,  some extended colors,
   and much other good work from wmcbrine over the last few years.

-  See history entries for versions 3.8 and 3.9 below for details.

-  Added chasonr's DOSVGA port and Federico G. Benavento and Jens
   Staal's Plan 9 port.

-  Automated builds again work and have been extended to new platforms.

-  Added ncurses-style init_extended_color(), init_extended_pair(),
   extended_color_content(),  extended_pair_content() functions.
   VT and WinGUI versions can now access 2^20 = over a million color
   pairs and 2^24+256 = 16777472 colors.  This should be extended
   to SDLx and X11,  and possibly WinCon.

-  As a result of all this,  A_RGB is now completely gone (and not
   really needed anyway).

-  Attempts to mis-link a wide-character library to code compiled for
   8-bit characters,  or a mismatch of a 32-bit chtype library to
   64-bit chtype code,  etc.,  will usually result in an undefined
   variant of initscr.  (Previously,  everything would compile,  then
   crash or produce odd output when the program was run.)  See GitHub
   issue #133.

-  Added 'picsview' demo,  something of a ripoff/homage to Thomas
   E. Dickey's 'picsmap' demo for ncurses.  It has a similar purpose:
   demonstrating the functions in the above paragraph and the new
   ability to access lots and lots of colors.

-  HISTORY.md brought up to date for 4.2 (which meant bringing in
   changes for 3.8 and 3.9).  Also,  4.1.0 was almost completely
   undocumented.

Minor new features
------------------

-  Name changed from 'PDCurses' to 'PDCursesMod',  in hopes of
   simultaneously making its origin clear and also making it clear
   that it's somewhat modified from the parent project.

-  Makefiles are better organized and more 'standard'.  'make' builds
   the PDCurses library,  'make demos' all demos.  The .mif files are
   all now in the 'common' directory.

-  VT port is much faster,  mostly thanks to not writing data to
   stdout with no buffering and with printf().  It also will use
   SGR mouse commands if available,  which makes things more bullet
   proof in cases of confused mouse input,  and means windows can
   be more than 224 columns wide without crashes happening when
   the mouse reaches that column.

-  Added ncurses extension functions ceiling_panel() and
   ground_panel().

-  'speed' demo tests out speed of the code.

-  'test_pan' allows one to move panels around the screen and adjust
   the panel depth.

-  Added ability to change SDL1/SDL2 font rendering mode by setting
   pdc_sdl_render_mode

Bug fixes and such
------------------

-  Valgrind found three small memory leaks in the VT port (which
   would apply to all other ports as well).  There are still more
   leaks in other ports,  which should be addressed.

-  Double-clicking in WinGUI got you a click message followed by
   a double-click.  This does conform to 'standard' Windows practice,
   but was both stupid and did _not_ conform with the way VT did
   it,  nor the way I'd want it to be on any port.  To do : enable
   double and triple clicks on all ports,  not just VT and WinGUI.

-  Handling of default background/foreground in VT was just plain
   wrong.  Now fixed.

-  Mouse wheel events now report the correct mouse position,  instead
   of always reporting (-1, -1).

-  Fixed spurious resize events at startup and for window moves on
   the X11 platform.

-  raw() and noraw() now work as specified on WinGUI and VT.  (They
   still don't work on X11,  SDLn,  or WinCon.)

-  User shrinking of the screen to a degree causing a window to be
   partly off-screen caused a crash.  (In PDCurses,  windows are
   theoretically always on-screen;  we were not properly prepared
   for the possibility that the screen might shrink and put windows
   partly or totally off-screen.)  We no longer crash,  and the part
   of the window that is still on-screen is refreshed correctly.

See the git log for more details.

PDCurses 4.1.0 - 2019-05-10
=========================

New features
------------

- VT backend improved to support different cursors,  mouse move events,
  and Ctrl/Alt/Shift flags.

- Fixes to allow WinCon builds on platforms lacking INFOEX struct.

- Improved DOS napms to convert milliseconds to BIOS ticks accurately.

- Followed wmcbrine's 'rebranding' of Public Domain Curses to PDCurses.

See the git log for more details.

PDCurses 4.0.4 - 2019-01-20
=========================

Major new feature:
-------------------

- New VT backend.  Works within an xterm or xterm-based terminal and some
  other terminals.  Mostly works in the Linux console,  and on Win10
  directly and on Win9x/DOS if ANSI.SYS (or NANSI or NNANSI .SYS or .COM)
  are installed.

Minor new features:
-------------------

- SDL2 variant: dimmed,  bold,  blinking,  and RGB text are handled,  as is
  window resizing.

- DOS variant: cross-compilation from GNU/Linux works with both DJGPP and
  Watcom C/C++; and, 16-bit and 32-bit Watcom makefiles have been combined
  into one

- Many modifications (and some bug fixes) taken from William McBrine's
  branch.  An effort has been made to narrow the gap between the forks
  where possible.  But both forks are moving targets with different
  design choices and priorities.

- Demos improved to show version info

Bug fixes
-------------------

- compilation warnings/errors with some compilers in some variants #53, #57, #58, #65, ...

- newtest sample was broken in all wide variants #60

- the paste button printed debug output #62

- some corner cases (midnight crossing, atomicity of tick count reads) in
  DOS version of napms() were not handled well

See the git log for more details.
------------------------------------------------------------------------

PDCurses 4.0.2 - 2017-09-12
=========================

Major new features:
-------------------

- New Win32a(Windows GUI) and SDL2 backends.  SDL1 is still supported,
  but may eventually go away.

- Bold, italic, underlined, overlined, dimmed, 'strikeout', blinking
  text, 256 colors and color pairs, and full RGB colors.  These are
  all supported in Win32a and mostly supported in X11, SDL1 and SDL2.

- In Win32a, one can choose a font, and both programmatic and user
  resizing are supported.  (Recompiling is necessary to change the
  font in X11.)

- (Win32a only) Support of SMP Unicode (points beyond 64K) and
  combining characters.  This may be extended to X11 and SDL2 eventually.

- Demos corrected to be buildable and testable with ncurses.

Minor new features:
-------------------

  (Note that not all of these are available on all backends)

- Support for up to nine mouse buttons and wheel and tilt-wheel mice,
  and double and triple mouse clicks.

- (X11, Win32a, Win32) Extended range of keys that are recognized.
  This now includes most of the "oddball" keys such as 'browser back
  and 'favorites' found on some keyboards.

- Blinking cursors in Win32a and X11 of various shapes (this could be
  extended to SDLx eventually).

- In X11 and Win32a, one can call resize_term( ) before initscr( ) to
  set the initial window size.

- Soft Label Keys (SLKs) are considerably more flexible, with the
  ability to set arbitrary numbers of keys and groupings.   See slk.c
  for details. This applies to all backends.

- Many changes to testcurs to test all these new features, and newtest
  added to test still more features.

- Option to get detailed version information of the used PDCurses
  library at run time with new exported PDC_version as PDC_version_info
  structure.

- ACS_x and WACS_x #defines extended to include a lot of "alternative
  characters" that overlap in Unicode and CP-437: double-line box chars,
  card suits, vulgar fractions, etc.  This applies to all backends. See
  acs_defs.h for the full list.

- Cleaned up some makefiles for Win32 and Win32a.  On both platforms,
  vcwin32.mak can now be used with the Intel(R) compiler, and
  mingwin32.mak can be used to cross-compile from Linux, or in
  command.com under Windows, or with Cygwin/MSYS.  Also added a
  makefile for Digital Mars for the DOS version.

- The "def" files that were needed before to create PDCurses on
  Windows are removed as they are no longer necessary.

See the git log for more details.

PDCurses 3.9 - 2019-09-04
=========================

IMPORTANT NOTE: This is actually wmcbrine's 3.9 version.  Its
changes did not actually make it into this fork until 4.1 and 4.2.

768 colors, single-process X11, copy-and-paste for all, and more.

New features
------------

- Single-process, single-thread version of the X11 port. Much, much
  faster than the two-process version. Needs more testing. This version
  omits translations.

- A common copy-and-paste system for all platforms, based on the
  PDC_*clipboard() functions. (This is the first time copy-and-paste is
  available for the SDL ports, and it replaces the old X11-specific
  C&P.) Press and hold button 1 while selecting; paste with button 2.
  Add Shift if mouse events are activated in curses. You can also paste
  via Shift-Ctrl-V, and copy with Shift-Ctrl-C (although selecting
  already sets the buffer). Note that paste is implemented via
  ungetch(), and is currently limited to 256 characters at a time. (You
  can get more via PDC_getclipboard().) With some ports (e.g. Wincon),
  the existing terminal C&P mechanism may override PDCurses'. DOS and
  SDL1 can only C&P within the same app.

- A new maximum of 768 colors, for Wincon, SDL and X11. COLOR_PAIRS is
  still limited to 256. The idea is that each pair can have a unique
  foreground and background, without having to redefine any of the first
  256 (predefined) colors. Colors 256-767 have no initial definitions,
  and are intended to be set via init_color(). An example has been added
  to testcurs (loosely based on part of newtest, by Bill Gray).

- Wincon now allows redefinition of all 768 colors, and allows it even
  under ConEmu.

- True italics for ConEmu. (It seems it should also support true bold,
  but I couldn't make that work.)

- Added new functions from ncurses and/or NetBSD: has_mouse(),
  is_keypad(), is_leaveok(), is_pad(), set_tabsize(), touchoverlap(),
  underscore(), underend(), wunderscore(), and wunderend(). See the man
  pages for descriptions. Partly due to Karthik Kumar Viswanathan, and
  suggestions of Simon Sobisch.

Bug fixes and such
------------------

- Check for standard C++ (>= 98), where native bool should exist, and use
  that; otherwise (pre-/non-standard C++) fall back to the old behavior.
  Satisfies clang, hopefully doesn't mess anything else up.

- Recent versions of clang throw an error over "-install_name".

- Most curses functions assumed a valid SP (i.e. that initscr() had
  already been called). Now, instead, they return ERR or other
  appropriate values. Suggestion of S.S.

- Deprecated PDC_save_key_modifiers() -- there's no benefit to NOT
  saving them.

- Hold back screen updates due to palette changes until paused; always
  do this update now (previously only in X11 and SDL, seems necessary in
  Windows 10 1903).

- SDL2 windows were freezing on moving to another screen (reported by
  Mark Hessling). Still issues with moving between screens of different
  scaling.

- Find the X libraries in some additional locations. After M.H.

- Converted default X11 icons to XPM, fixing their non-display in Ubuntu.

- Made XIM standard, removed "classic" X11 compose system.

- Made wide-character build the default for X11 (--disable-widec for
  narrow).

- Smoother resizing in X11, when not in scrollbar mode.

- Dropped X11 options "borderWidth" (broken since at least 2.7) and
  "cursorColor" (now set automatically for contrast).

- Correctly restore Insert mode and QuickEdit mode in Wincon's
  PDC_reset_shell_mode(). Patch by "vyv03354".

- Add a WINDRES variable to wincon/Makefile for the sake of cross-
  compilers. Patch by Marc-Andre Lureau.

- Suppress cursor movement during color tests in testcurs.

- Added UTF-8-demo.txt for tuidemo to browse (by default, only in forced
  UTF-8 mode). File by Markus Kuhn.

- Moved the doc files from "man" to "docs" -- the docs/man thing was too
  confusing. Streamlined the web page into two files.

- Rewrote the "Portability" sections of the man pages to reflect current
  ncurses and NetBSD. The old charts weren't very accurate.

- Document resolution of timeout() and napms(). Suggested by S.S.

- Rewrote manext (again) in Awk.

- Changed most dates to ISO format.

See the git log (for both PDCursesMod and for wmcbrine's PDCurses,  the
original source of these changes) for more details.

------------------------------------------------------------------------

PDCurses 3.8 - 2019-02-02
=========================

IMPORTANT NOTE: This is actually wmcbrine's 3.8 version.  Its
changes did not actually make it into this fork until 4.1.

It's that time again.


New features
------------

- PDC_VERSION structure and PDC_get_version() function, to provide run-
  time information on version and compile options, in case they don't
  match the header; along with new compile-time defines PDC_VER_MAJOR,
  PDC_VER_MINOR and PDC_VERDOT. Suggested by Simon Sobisch, designed
  partly after Bill Gray and partly after SDL_VERSION.

- Extensive documentation revisions, now covering many previously
  undocumented functions.

- Allow building the DLL with MinGW for SDL. (This also changes the
  non-DLL library name from libpdcurses.a to pdcurses.a.)

- Consolidated Watcom makefiles for DOS, after Tee-Kiah Chia; added
  MODEL option to Makefile.bcc for consistency.

- Added another ncurses_test, "lrtest"; updated for ncurses 6.1.


Bug fixes and such
------------------

- T.H.'s update rect clipper (a resize fix for SDL2) broke sdltest,
  because it didn't take the offsets into account for a non-owned
  window.

- The version number is now hardwired only in curses.h and configure.ac.

- Revised pdcurses.rc to correctly show all fields when checking the
  properties on a DLL; use it with MinGW as well as MSVC.

- Allow building both 32- and 64-bit SDL2 versions in MinGW without
  editing the Makefile, by using the proper dev package.

- Build SDL2 demos in "Windows" mode (i.e. no controlling terminal) with
  MSVC, as with MinGW.

- Build sdltest.exe with MSVC.

- Changed sample pathname in tuidemo to always use slashes -- the
  backslashes failed in, e.g., SDL under Linux or macOS. Patch by B.G.

- Warning fix for Borland OS/2.

- Minor file reorganization / renaming.

- mmask_t is now used in both the classic and ncurses mouse interfaces,
  and is defined in such a way as to keep it at 32 bits.

- Dropped map_button() and getbmap().

- Dropped the ability to build BBS-ready archives from the Makefiles.

- Made manext.py compatible with Python 3.x.

See the git log (for both PDCursesMod and for wmcbrine's PDCurses,  the
original source of these changes) for more details.

PDCurses 3.7 - 2018-12-31
=========================

New features
------------

- Avoid conflict with ncurses by having apps define PDC_NCMOUSE before
  including curses.h to invoke the ncurses-style mouse interface,
  instead of NCURSES_MOUSE_VERSION. (The old way will also still work.)
  After Simon Sobisch (see PR #33).

- In SDL (TTF mode), the box-drawing and block ACS characters are now
  rendered in a font-independent way, to ensure their correct alignment
  across cells. Underlining is now handled in a similar way.

- TTF fonts in SDL are now rendered in Blended mode instead of Solid.
  Partly after Joachim de Groot.

- New default fonts and font sizes for SDL/TTF.

- SDL2 now builds under MSVC. Partly due to Alexandru Afrasinei.

- Documentation re-org -- more Markdown internal links; moved to man/
  dir (the doc/ dir name was too similar to docs/, which is needed for
  GitHub Pages hosting); concatenated man page document now made
  permanent, under the name MANUAL.md; new man build utils; merged
  sdl.md and x11.md into their respective READMEs; changed some
  redundant and unclear comments.

- Directory re-org -- in addition to the above, created common/, to
  unclutter the root, and eliminate a few more redundant files from
  platform directories. (We already had "pdcurses", but that's for the
  portable core; "common" is for files that are more platform-specific,
  though shared by more than one platform.)

- Broke out the redundant ACS tables and moved them to common/.

- PDcurses' "bool" type is now based on stdbool.h, when available. There
  should be no conflicts when including stdbool.h either before or after
  curses.h.

- The demos are no longer built by default, since they add a lot of time
  to the build, and often aren't wanted. But you can still build them via
  "make demos" (tweak as needed).

- Makefile tweaks for cross-compiling by Simon Sobisch.


Bug fixes and such
------------------

- Improved Windows console resizing, when reducing the vertical size.
  After Ulf Magnusson. (See GitHub issue #26.)

- Bring back ifdef'd CONSOLE_SCREEN_BUFFER_INFOEX, for the benefit of
  older compile environments. (Not automatic -- must specify INFOEX=N on
  the command line.) After Simon Sobisch.

- Replaced COMMON_LVB_* with numbers to appease some old compilers.
  After Simon Sobisch.

- KEY_RESIZE should be key_code = TRUE. Reported by Ulf Magnusson.

- SDL2 resize fixes to prevent crashes, by Tim Hentenarr.

- SDL2 fixes for handling of SDL_TEXTINPUT, keys with modifiers, and
  modifier keystrokes, by Tim Hentenarr.

- Fixed cursor rendering in SDL/TTF.

- SDL1 support is now dropped for Windows and macOS, and deprecated for
  Linux. Use SDL2. The SDL1 port is likely to be dropped in the future.

- The setsyx() function is now void, after ncurses, and simplified.

- Warning fixes by Patrick Georgi and Stefan Reinauer.

- X11 used SP->resized in a non-boolean way, so it's now a short.

- Under some conditions (see issue #47), the X11 port could "free" colors
  that it hadn't allocated. Reported by rin-kinokocan.

- New scroller for ozdemo -- no memory allocation, less copying -- to
  resolve issue #46.

- Various minor Makefile tweaks.

- Eliminated term.h and terminfo.c, and moved mvcur() to move.c. These
  stub functions, done on request (with others then requesting that I
  take them away -- can't win), were a misguided attempt to facilitate
  using PDCurses with certain non-C languages -- which, apparently, they
  didn't end up actually doing. They're also, regrettably, specified as
  part of the X/Open curses standard, even though they in effect
  describe an entirely different interface layer (one on which
  traditional curses, but not PDCurses, is built).

- Dropped support for short (16-bit) chtypes.

- Finally removed deprec.c, as it promised.

- Dropped the XOPEN, SYSVcurses and BSDcurses defines from curses.h, as
  well as NULL (which is defined in stdio.h, included). TRUE, FALSE, ERR
  and OK are now defined unconditionally.

- Moved pdcurses.org hosting to GitHub -- as a result, the site is now
  part of the repo, in the docs/ directory. (Also, it has SSL again.)

See the git log for more details.

------------------------------------------------------------------------

PDCurses 3.6 - 2018-02-14
=========================

[Note : this is a copy of wmcbrine's update history for that version.
The changes involved have,  however,  now been integrated into
this fork.]

Tidying up some loose ends from 3.5, and trying to bring all platforms
up to the same level, as much as possible.

New features
------------

- 256 colors for the Windows console -- under Windows 10 or ConEmu,
  only. This version doesn't allow init_color() or color_content() for
  colors 16-255, just uses Windows' predefined palette (which matches
  xterm-256color, like the default colors in X11 and SDL).

- Real blinking for the Windows console (all), and for OS/2 -- done in
  software, like the Windows version -- replacing the erraticly working
  Vio-based version (which didn't work at all in my OS/2 4.5 VM). OS/2
  now always has 16 colors, and bright backgrounds can combine with
  blinking.

- In DOS, OS/2 and Windows, attribute behavior now more closely matches
  that of the more "advanced" ports (X11 and SDL) -- see the Attribute
  test in testcurs.

- All of the A_* and WA_* attributes from X/Open are now defined in
  curses.h, although some are no-ops, pending the availablity of more
  attribute bits. A_INVIS is now a no-op on all platforms, instead of
  overloading A_ITALIC, and so is A_PROTECT. A_LEFT and A_RIGHT are now
  synonyms for PDCurses' old *LINE attributes.


Bug fixes and such
------------------

- For the X11 port, "make install" and the dynamic library build were
  broken, since the configure move. Fixes partly after Mark Hessling.

- Renamed "win32" to the more accurate/descriptive "wincon" (i.e.
  WINdows CONsole). Makefiles for all platforms renamed to remove the
  redundant platform names, and to allow better sorting.

- In SDL2, apps that didn't explicitly handle resizing locked up. Now,
  they can continue running, at their old size. (To Do: xmas is still a
  basket case.)

- Added "/MACHINE:$(PLATFORM)" to wincon/Makefile.vc -- Thomas Dickey
  says this is needed to build 64-bit with Visual Studio Express 2012.
  With 2017, it suppresses a warning.

- Suppressed "Possibly incorrect assignment" warnings with BCC, which
  also results in more readable code.

- Cleaned up obsolete comments, dead code, unneeded includes, typos, and
  outdated documentation.

- Dropped support for EMXVIDEO.

- Dropped color remapping for OS/2 (broken).

- Dropped X11 DLL support for Cygwin (broken).

- Rearranged extended color display in testcurs.

- In ptest, handle resizing, and check for screens too small to run in.

- Allow KEY_* codes (including KEY_RESIZE) to exit firework, as other
  keys do.

- Slightly faster Windows compilation (most noticeable in Watcom).

See the git log for more details.

------------------------------------------------------------------------

PDCurses 3.5 - 2018-01-15
=========================

So, it's been a while, eh?

This release is an attempt to bring PDCurses mostly up to date, without
breaking too many things in the process.


New features
------------

- SDL2 port, and TTF and Unicode support for both SDL1 and SDL2. Credit
  for these goes mostly to Laura Michaels and Robin Gustafsson.

- 256 colors for SDL and X11, by Bill Gray. Colors 16-255 are set up to
  match xterm colors, but can be redefined, as with 0-15.

- Bold and italic font options for SDL and X11. A_BOLD's behavior is
  controlled by the new function PDC_set_bold() -- TRUE to select bold
  font, FALSE to choose high foregound intensity (as before). Italic
  fonts are selected by A_ITALIC (always on). X11 originally from Mark
  Hessling.

- Real blinking in SDL and X11, controlled by PDC_set_blink(). Largely
  due to Kevin Lamonte and Bill Gray.

- Support for A_UNDERLINE, A_LEFTLINE and A_RIGHTLINE in the Windows
  console. This requires a recent version of Windows (10, maybe 8?) to
  work with the standard console, but underlining also works with
  ConEmu, at least as far back as XP.

- User resizing (i.e. grab window edges or maximize button) for Windows
  console -- needs recent Windows or ConEmu.

- New-style color-changing code for the Windows console (using the new
  offical API instead of undocumented functions), supporting
  redefinition of colors 0-15 via init_color(). Works at least as far
  back as Windows XP SP3. Patch by "Didrole".

- The Windows console port now creates a separate console buffer by
  default, making for a cleaner and more complete restoration of the
  original buffer. The old behavior can be used by setting
  "PDC_RESTORE_SCREEN=0". Patch by Jason Hood.

- Left/right scroll wheel support for Windows console, SDL and X11. X11
  by Mark Hessling.

- testcurs now includes an additional test to show various attributes,
  and a display of the extended colors, where applicable.


Bug fixes and such
------------------

- termattrs() now returns something vaguely resembling the actual
  capabilities of the specific "terminal". Specifically, A_BOLD and
  A_BLINK reflect the availability of true bold fonts, and real
  blinking; when not set in termattrs(), the attributes still work, but
  control foreground and background intensity, as before. *LINE are also
  meaningful, and even A_COLOR is set (or not).

- pad size check in pnoutrefresh() was broken since 3.0. Reported by
  Peter Hull.

- In newpad(), begx and begy should be set to zero, otherwise creating a
  subpad of the same width or height fails due to the check in subpad().
  Patch by Raphael Assenat.

- More straightforward math for subpad(), plus another off-by-one error.
  Reported by Werner Wenzel, John Hartmann et al.

- New subwindows/subpads/resized windows should copy _delayms. Patch by
  "xaizek".

- Potentially invalid saved cursor position in resize_window() --
  another off-by-one _maxx/_maxy error. Patch after "Luke-Jr".

- copywin() needs to disallow corner values equal to _maxx or _maxy, not
  just less than. Reported by "Aleksandr".

- Misaligned soft-label keys in 4-4-4 mode. Reported by Werner Wenzel.

- Missing prototypes for bkgrnd() and bkgrndset().

- Missing WA_NORMAL and WA_ATTRIBUTES from the X/Open spec.

- keyname() and termname() now return static buffers, as documented.

- In the X11 port, due to (post-PDCurses-3.4) changes in Xt,
  XtAppMainLoop() always hung. Fixed by re-implementing it within
  PDCurses, basically.

- Fix blinking X11 cursor for clients that call move() more frequently
  than cursorBlinkRate -- patch by Kevin Lamonte.

- Improved cursor rendering for X11, by John P. Hartmann.

- ALT key combos sometimes not reported in X11, per Mark Hessling et al.

- Support for XK_ISO_Left_Tab in X11, by John P. Hartmann.

- Support for "Super" keys in X11, by Bill Gray.

- Make xcurses-config include -DPDC_WIDE when appropriate, per M.H.

- The configure script and accompanying files, which were always
  specific only to the X11 port (causing considerable confusion), have
  been moved to the x11 directory.

- In SDL, SP->key_code wasn't being set for KEY_MOUSE events. Reported
  by Bill Gray.

- SDL events need to keep pumping through non-input delays. (Really
  messed up on current macOS before this change.)

- SDL2 is outperforming SDL1 by about 10x on the platforms I've tried
  that support both, so I've removed Makefile.mng from the SDL1 port.

- Updated for the most current compilers, wherever possible; various
  warning suppressions. All included makefiles were tested with their
  respective compilers, shortly before release (including the POSIX
  stuff on macOS with clang, and on Ubuntu Linux with gcc). The oldest
  compiler I tested with was Turbo C++ 3.0, from 1992; the latest,
  several compilers from 2017.

- Dropped support for LCC-Win32 -- the official site is shut down.

- Dropped support for Digital Mars -- not updated since 2014, limited
  makefile, library missing some needed Windows APIs.

- Dropped MS C for DOS, and Cset/2 for OS/2.

- Dropped support for building DLLs with EMX.

- Minor code and makefile reorganization; mingwin32.mak merged into
  gccwin32.mak (i.e. you can use it with both compilers). Some
  contributions by Bill Gray and Simon Sobisch.

- Watcom makefile paths and option markers changed to Unix-friendly
  style, after Tee-Kiah Chia.

- The *.def files are no longer needed, replaced by more PDCEX
  declarations in the include files. After Bill Gray and Simon Sobisch.

- When building with DEBUG=Y, no longer strip the executables. After
  Simon Sobisch.

- Hold debug file ("trace") open after traceon(), for greater
  performance. Set PDC_TRACE_FLUSH to make it fflush() after each write
  (slower but safer in case of a crash). Patch by Ellie Timoney.

- Since 3.2, the panel library was simply a copy of the main library.
  This kludge is now dropped. (panel.h remains separate from curses.h.)

- Removed PDCurses.spec, and the RPM-building makefile option. I think
  this is better left to the various package/distro maintainers.

- Various formatting corrections (e.g., trailing spaces stripped), and
  variables renamed to avoid clashes. Some contributed by Stefan
  Reinauer and Bill Gray.

- Various documentation corrections and updates. All documentation
  "converted" to Markdown format (involving few actual changes -- mainly
  the file extension), for better rendering on GitHub, SourceForge, etc.
  Some contributed by Anatoly Techtonik.

- The "Win32" label is deprecated by Microsoft, and accordingly I've
  replaced references in the documentation, although not yet changed the
  filenames. The Windows console code can just as well be built for
  64-bit (and always could be, AFAIK, although there are minor tweaks
  to support it in this version).

- The ncurses_tests can now be built under SDL as well as X11. Also, all
  our tests (still/again) build under recent ncurses.

- Put testcurs' "Output test" into real blink mode, if possible; and if
  COLORS >= 16, use colors 0-15 directly in the color test, instead of
  or'ing with A_BOLD to get the high-intensity colors.

- Renamed the (by now rather old) "newdemo" to "ozdemo".

- Moved from CVS to git; source is now on GitHub as well as SourceForge;
  central site is now pdcurses.org.

See the git log for more details.

------------------------------------------------------------------------

PDCurses 4.0.2 - 2017 Sep 12   (Bill Gray fork)
=========================
   (Note that history gets confused here;  we have things happening
in two forks.)

Major new features:
-------------------

- New WinGUI (Windows GUI) and SDL2 backends.
  SDL1 is still supported, but may eventually go away.

- Bold, italic, underlined, overlined, dimmed, 'strikeout', blinking text,
  256 colors and color pairs,  and full RGB colors.
  These are all supported in WinGUI and mostly supported in X11, SDL1 and SDL2.

- In WinGUI, one can choose a font, and both programmatic and user resizing
  are supported.
  (Recompiling is necessary to change the font in X11.)

- (WinGUI only) Support of SMP Unicode (points beyond 64K) and combining
  characters.
  This may be extended to X11 and SDL2 eventually.

- Demos corrected to be buildable and testable with `ncurses`.

Minor new features
-------------------
(note that not all of these are available on all backends)

- Support for up to nine mouse buttons and wheel and tilt-wheel mice, and
  double and triple mouse clicks

- (X11, WinGUI, Win32) Extended range of keys that are recognized. This
  now includes most of the "oddball" keys such as 'browser back' and
  'favorites' found on some keyboards.

- Blinking cursors in WinGUI and X11 of various shapes (this could be
  extended to SDLx eventually).

- In X11 and WinGUI, one can call resize_term( ) before initscr( ) to set
  the initial window size.

- Soft Label Keys (SLKs) are considerably more flexible,  with the ability
  to set arbitrary numbers of keys and groupings.
  See slk.c for details. This applies to all backends.

- Many changes to `testcurs` to test all these new features, and `newtest`
  added to test still more features.

- Option to get detailed version information of the used PDCurses library
  at run time with new exported `PDC_version` as `PDC_version_info` structure.

- ACS_x and WACS_x #defines extended to include a lot of "alternative
  characters" that overlap in Unicode and CP-437:  double-line box chars,
  card suits,  vulgar fractions,  etc.
  This applies to all backends.  See `acs_defs.h` for the full list.

- Cleaned up some makefiles for Win32 and WinGUI.
  On both platforms, `vcwin32.mak` can now be used with the Intel(R) compiler,
  and `mingwin32.mak` can be used to cross-compile from Linux, or in
  `command.com` under Windows, or with Cygwin/MSYS.
  Also added a makefile for Digital Mars for the DOS version.

------------------------------------------------------------------------

PDCurses 3.4 - 2008-09-08
=========================

Nothing much new this time, but I've been sitting on some bug fixes for
almost a year, so it's overdue. Apart from bugs, the main changes are in
the documentation.

New features:

- setsyx() is now a function rather than a macro.

Bug fixes and such:

- In x11, the xc_atrtab table size was under-calculated by half,
  resulting in crashes at (oddly) certain line counts. (It should've
  crashed a lot more.) Reported by Mark Hessling.

- Test for moved cursor was omitting the window origin offset. Reported
  by Carey Evans.

- Is DOS and OS/2, the value for max items in key_table was still wrong.
  Reported by C.E.

- Changed isendwin() so it won't crash after delscreen().

- Ensure zero-termination in PDC_mbstowcs() and PDC_wcstombs().

- Disable QuickEdit Mode when enabling mouse input for the Win32
  console; reported by "Zalapkrakna".

- Fix for building under Innotek C (I hope). Report by Elbert Pol, fix
  courtesy of Paul Smedley.

- Unified exports list with no duplicates -- pdcurses.def is now built
  from components at compile time.

- Don't install curspriv.h, and don't include it with binary
  distributions.

- Building DLLs with LCC is no longer supported, due to the primitive
  nature of its make.exe.

- Export the terminfo stub functions from the DLLs, too.

- Added support for Apple's ".dylib" in configure. Suggested by Marc
  Vaillant (who says it's needed with OS 10.5.)

- In sdl1/Makefile.mng, ensure that CC is set.

- In the gcc makefiles, "$?" didn't really have the desired effect --
  _all_ the dependencies showed up on the command line, including
  curses.h, and pdcurses.a twice.  And apparently, this can mess up some
  old version (?) of MinGW. So, revert to spelling out "tuidemo.o
  tui.o". Reported by "Howard L."

- Extensive documentation revision and reorganizing. More to do here.
  For example, I moved the build instructions from INSTALL (which never
  really described installation) to the platform-specific READMEs.

- New indentation standard: four spaces, no tabs.

------------------------------------------------------------------------

PDCurses 3.3 - 2007-07-11
=========================

This release adds an SDL backend, refines the demos, and is faster in
some cases.

New features:

- SDL port. See INSTALL, doc/sdl.txt and sdl1/* for details.

- Double-buffering -- minimize screen writes by checking, in doupdate()
  and wnoutrefresh(), whether the changes to curscr are really changes.
  In most cases, this makes no difference (writes were already limited
  to areas marked as changed), but it can greatly reduce the overhead
  from touchwin(). It also helps if you have small, separated updates on
  the same line.

- The PDC_RGB colors can now be used, or not, with any platform (as long
  as the same options are used when compiling both the library and
  apps). This may help if you have apps that are hardwired to assume
  certain definitions.

- Restored the use_default_colors() stuff from the ncurses versions of
  the rain and worm demos, to make them "transparent" (this is useful
  now, with the SDL port); added transparency to newdemo.

- Added setlocale() to tuidemo, to make it easier to browse files with
  non-ASCII characters.

- Sped up firework demo by replacing unneeded clear() and init_pair()
  calls.

- Allow exit from ptest demo by typing 'q'.

- New functions for implementors: PDC_pair_content() and PDC_init_pair()
  (the old pdc_atrtab stuff was arguably the last remnant of code in the
  pdcurses directory that was based on platform details).

Bug fixes and such:

- Implicit wrefresh() needs to be called from wgetch() when the window's
  cursor position is changed, even if there are no other changes.

- Set SP->audible on a per-platform basis, as was documented in
  IMPLEMNT, but not actually being done.

- Minor tweaks for efficiency and readability, notably with wscrl().

- tuidemo didn't work correctly on monochrome screens when A_COLOR was
  defined -- the color pair numbers appeared as the corresponding
  character; also, the input box was (I now realize) broken with ncurses
  since our 2.7, and broke more subtly with PDCurses' new implicit
  refresh handling; also, the path to the default file for the Browse
  function was a bit off.

- Assume in the demos that curs_set() is always available -- there's no
  good test for this, and the existing tests were bogus.

- Made the command-line parameter for ptest work. (If given an argument,
  it delays that number of milliseconds between changes, instead of
  waiting for a key, and automatically loops five times.)

- Building the Win32 DLL with MinGW or Cygwin wouldn't work from outside
  the platform directory.

- Building the X11 port with Cygwin required manually editing the
  Makefile after configuring; no longer. Reported by Warren W. Gay.

- Minor tightening of configure and makefiles.

- Bogus references to "ACS_BLCORNER" in the border man page. Reported by
  "Walrii".

- slk_wlabel() was not documented.

- Spelling cleanup.

- Changed RCSIDs to not end with a semicolon -- avoids warnings when
  compiling with the -pedantic option.

- Merged latin-1.txt into x11.txt.

- Updated config.guess and config.sub to more recent versions.

------------------------------------------------------------------------

PDCurses 3.2 - 2007-06-06
=========================

This release mainly covers changes to the build process, along with a
few structural changes.

New features:

- The panel library has been folded into the main library. What this
  means is that you no longer need to specify "-lpanel" or equivalent
  when linking programs that use panel functionality with PDCurses;
  however, panel.lib/.a is still provided (as a copy of pdcurses.lib/.a)
  so that you can, optionally, build your projects with no changes. It
  also means that panel functionality is available with the DLL or
  shared library. Note that panel.h remains separate from curses.h.

- Setting the PDCURSES_SRCDIR environment variable is no longer required
  before building, unless you want to build in a location other than the
  platform directory. (See INSTALL.)

- MinGW and Cygwin makefiles support building DLLs, via the "DLL=Y"
  option. Partly due to Timofei Shatrov.

- Support for the Digital Mars compiler.

- Watcom makefiles now use the "loaddll" feature.

Bug fixes and such:

- Eliminated the platform defines (DOS, WIN32, OS2, XCURSES) from
  curses.h, except for X11-specific SCREEN elements and functions.
  Dynamically-linked X11 apps built against an old version will have
  their red and blue swapped until rebuilt. (You can define PDC_RGB to
  build the library with the old color scheme, but it would also have to
  be defined when building any new app.) Any app that depends on
  PDCurses to determine the platform it's building on will have to make
  other arrangements.

- Documentation cleanup -- added more details; removed some content that
  didn't apply to PDCurses; moved the doc-building tool to the doc
  directory; changed *.man to *.txt.

- The EMX makefile now accepts "DLL=Y", builds pdcurses.dll instead of
  curses.dll, builds either the static library or the DLL (not both at
  once), and links all the demos with the DLL when building it.

- In Win32, read the registry only when needed: when init_color() or
  color_content() is called, instead of at startup.

- A few additional consts in declarations.

- The Win32 compilers that build DLLs now use common .def files.

- panel.h functions sorted by name, as with other .h files; curses.h is
  no longer included by repeated inclusions of panel.h or term.h.

- Simplified Borland makefiles.

- Makefile.aix.in depended on a file, xcurses.exp, that was never there.
  This problem was fixed as part of the change to common .def files;
  however, I still haven't been able to test building on AIX.

------------------------------------------------------------------------

PDCurses 3.1 - 2007-05-03
=========================

Primarily clipboard-related fixes, and special UTF-8 support.

New features:

- "Force UTF-8" mode, a compile-time option to force the use of UTF-8
  for multibyte strings, instead of the system locale. (Mainly for
  Windows, where UTF-8 doesn't work well in the console.) See INSTALL.

- Multibyte string support in PDC_*clipboard() functions, and in Win32's
  PDC_set_title().

- Added the global string "ttytype", per other curses implementations,
  for compatibility with old BSD curses.

- Real functions for the "quasi-standard aliases" -- crmode(),
  nocrmode(), draino(), resetterm(), fixterm() and saveterm().
  (Corresponding macros removed.)

Bug fixes and such:

- In Win32, under NT-family OSes, the scrollback buffer would be
  restored by endwin(), but would not be turned off again when resuming
  curses after an endwin(). The result was an odd, partly-scrolled-up
  display. Now, the buffer is toggled by PDC_reset_prog_mode() and
  PDC_reset_shell_mode(), so it's properly turned off when returning
  from an endwin().

- In 3.0, selection in X11 didn't work. (Well, the selecting worked, but
  the pasting elsewhere didn't.) This was due to the attempted fix
  "don't return selection start as a press event," so that's been
  reverted for now.

- PDC_setclipboard() was locking up in X11. Reported by Mark Hessling.

- Missing underscore in the declaration of XC_say() prevented
  compilation with PDCDEBUG defined.  Reported by M.H.

- Off-by-one error in copywin() -- the maximum coordinates for the
  destination window should be inclusive. Reported by Tiago Dionizio.

- Start in echo mode, per X/Open. Reported by T.D.

- Strip leading and trailing spaces from slk labels, per a literal
  reading of X/Open. Suggested by Alexey Miheev (about ncurses, but it
  also applies here).

- The #endif for __PDCURSES__ needs to come _after_ the closing of the
  extern "C". This has been broken since June 2005. Fortunately (?), it
  only shows up if the file is included multiple times, and then only in
  C++. Reported on the DOSBox forums.

- Use CF_OEMTEXT instead of CF_TEXT in the narrow versions of the
  clipboard functions in Win32, to match the console.

- Changed the format of the string returned from longname().

- In the clipboard test in the testcurs demo, use a single mvprintw() to
  display the return from PDC_getclipboard(), instead of a loop of
  addch(), which was incompatible with multibyte strings.

- Moved has_key() into the keyname module, and documented it.

- Moved RIPPEDOFFLINE to curspriv.h.

- Typos in IMPLEMNT.

------------------------------------------------------------------------

PDCurses 3.0 - 2007-04-01
=========================

The focuses for this release are X/Open conformance, i18n, better color
support, cleaner code, and more consistency across platforms.

This is only a brief summary of the changes. For more details, consult
the CVS log.

New features:

- An almost complete implementation of X/Open curses, including the
  wide-character and attr_t functions (but excluding terminfo). The
  wide-character functions work only in Win32 and X11, for now, and
  require building the library with the appropriate options (see
  INSTALL). Note that this is a simplistic implementation, with exactly
  one wchar_t per cchar_t; the only characters it handles properly are
  those that are one column wide.

- Support for X Input Methods in the X11 port (see INSTALL). When built
  this way, the internal compose key support is disabled in favor of
  XIM's, which is a lot more complete, although you lose the box cursor.

- Multibyte character support in the non-wide string handling functions,
  per X/Open. This only works when the library is built with wide-
  character support enabled.

- Mouse support for DOS and OS/2. The DOS version includes untested
  support for scroll wheels, via the "CuteMouse" driver.

- An ncurses-compatible mouse interface, which can work in parallel with
  the traditional PDCurses mouse interface. See the man page (or
  mouse.c) for details.

- DOS and OS/2 can now return modifiers as keys, as in Win32 and X11.

- COLORS, which had been fixed at 8, is now either 8 or 16, depending on
  the terminal -- usually 16. When it's 8, blinking mode is enabled
  (controlled as before by the A_BLINK attribute); when it's 16, bright
  background colors are used instead. On platforms where it can be
  changed, the mode is toggled by the new function PDC_set_blink().
  PDCurses tries to set PDC_set_blink(FALSE) at startup. (In Win32, it's
  always set to FALSE; in DOS, with other than an EGA or VGA card, it
  can't be.) Also, COLORS is now set to 0 until start_color() is called.

- Corresponding to the change in COLORS, COLOR_PAIRS is now 256.

- Working init_color() and color_content(). The OS/2 version of
  init_color() works only in a full-screen session; the Win32 version
  works only in windowed mode, and only in NT-family OSes; the DOS
  version works only with VGA adapters (real or simulated). The Win32
  version is based mostly on James Brown's setconsoleinfo.c
  (www.catch22.net).

- use_default_colors(), assume_default_colors(), and curses_version(),
  after ncurses.

- Added global int TABSIZE, after ncurses and Solaris curses; removed
  window-specific _tabsize.

- Logical extension to the wide-character slk_ funcs: slk_wlabel(), for
  retrieving the label as a wide-character string.

- A non-macro implementation of ncurses' wresize().

- Working putwin(), getwin(), scr_dump() and scr_restore().

- A working acs_map[]. Characters from the ACS are now stored in window
  structures as a regular character plus the A_ALTCHARSET attribute, and
  rendered to the ACS only when displayed. (This allows, for example,
  the correct display on one platform of windows saved from another.)

- In X11, allow selection and paste of UTF8_STRING.

- The testcurs demo now includes a color chart and init_color() test, a
  wide character input test, a display of wide ACS characters with
  sample Unicode text, a specific test of flash(), more info in the
  resize test, and attempts to change the width as well as the height.

- Command-line option for MSVC to build DLLs (see INSTALL). Also, the
  naming distinction for DLLs ("curses" vs. "pdcurses") is abandoned,
  and either the static lib or DLL is built, not both at once (except
  for X11).

- For backwards compatibility, a special module just for deprecated
  functions -- currently PDC_check_bios_key(), PDC_get_bios_key(),
  PDC_get_ctrl_break() and PDC_set_ctrl_break(). These shouldn't be used
  in applications, but currently are... in fact, all the "private"
  functions (in curspriv.h) are subject to change and should be avoided.

- A new document, IMPLEMNT, describing PDCurses' internal functions for
  those wishing to port it to new platforms.

- Mark Hessling has released the X11 port to the public domain.
  (However, x11/ScrollBox* retain their separate copyright and MIT-like
  license.)

Bug fixes and such:

- Most of the macros have been removed (along with the NOMACROS ifdef).
  The only remaining ones are those which have to be macros to work, and
  those that are required by X/Open to be macros. There were numerous
  problems with the macros, and no apparent reason to keep them, except
  tradition -- although it was PCcurses 1.x that first omitted them.

- Clean separation of platform-specific code from the rest. Outside of
  the platform directories, there remain only a few ifdefs in curses.h
  and curspriv.h.

- General reorganization and simplification.

- Documentation revisions.

- When expanding control characters in addch() or insch(), retain the
  attributes from the chtype.

- Preserve the A_ALTCHARSET attribute in addch() and insch().

- Per X/Open, beep() should always return OK.

- On platforms with a controlling terminal (i.e., not X11), curs_set(1)
  now sets the cursor to the shape it had at the time of initscr(),
  rather than always making it small. (Exception for DOS: If the video
  mode has been changed by PDC_resize_screen(), curs_set(1) reverts to
  line 6/7.) The shape is taken from SP->orig_cursor (the meaning of
  which is platform-specific).

- Stop updating the cursor position when the cursor is invisible (this
  gives a huge performance boost in Win 9x); update the cursor position
  from curs_set() if changing from invisible to visible.

- Some tweaking of the behavior of def_prog_mode(), def_shell_mode(),
  savetty(), reset_prog_mode(), reset_shell_mode() and resetty()...
  still not quite right.

- flash() was not implemented for Win32 or X. A portable implementation
  is now used for all platforms. Note that it's much slower than the
  old (DOS and OS/2) version, but this is only apparent on an extremely
  slow machine, such as an XT.

- In getstr(), backspacing on high-bit characters caused a double
  backspace.

- hline() and vline() used an incorrect (off by one) interpretation of
  _maxx and _maxy. If values of n greater than the max were specified,
  these functions could access unallocated memory.

- innstr() is supposed to return the number of characters read, not just
  OK or ERR. Reported by Mike Aubury.

- A proper implementation of insch() -- the PDC_chadd()-based version
  wasn't handling the control characters correctly.

- Return ASCII and control key names from keyname() (problem revealed by
  ncurses' movewindow test); also, per X/Open, return "UNKNOWN KEY" when
  appropriate, rather than "NO KEY NAME".

- Turn off the cursor from leaveok(TRUE), even in X11; leaveok(FALSE)
  now calls curs_set(1), regardless of the previous state of the cursor.

- In the slk area, BUTTON_CLICKED events now translate to function keys,
  along with the previously recognized BUTTON_PRESSED events. Of course,
  it should really be checking the events specified by map_button(),
  which still doesn't work.

- napms(0) now returns immediately.

- A unified napms() implementation for DOS -- no longer throttles the
  CPU when built with any compiler.

- Allow backspace editing of the nocbreak() buffer.

- pair_content(0, ...) is valid.

- There was no check to ensure that the pnoutrefresh() window fit within
  the screen. It now returns an ERR if it doesn't.

- In X11, resize_term() must be called with parameters (0, 0), and only
  when SP->resized is set, else it returns ERR.

- Copy _bkgd in resize_window(). Patch found on Frederic L. W. Meunier's
  web site.

- slk_clear() now removes the buttons completely, as in ncurses.

- Use the current foreground color for the line attributes (underline,
  left, right), unless PDC_set_line_color() is explicitly called. After
  setting the line color, you can reset it to this mode via
  "PDC_set_line_color(-1)".

- Removed non-macro implementations of COLOR_PAIR() and PAIR_NUMBER().

- Dispensed with PDC_chadd() and PDC_chins() -- waddch() and winsch()
  are now (again) the core functions.

- Dropped or made static many obsolete, unused, and/or broken functions,
  including PDC_chg_attrs(), PDC_cursor_on() and _off(),
  PDC_fix_cursor(), PDC_get_attribute(), PDC_get_cur_col() and _row(),
  PDC_set_80x25(), PDC_set_cursor_mode(), PDC_set_rows(),
  PDC_wunderline(), PDC_wleftline(), PDC_wrightline(),
  XCursesModifierPress() and XCurses_refresh_scrollbar().

- Obsolete/unused defines: _BCHAR, _GOCHAR, _STOPCHAR, _PRINTCHAR
  _ENDLINE, _FULLWIN and _SCROLLWIN.

- Obsolete/unused elements of the WINDOW struct: _pmax*, _lastp*,
  _lasts*.

- Obsolete/unused elements of the SCREEN struct: orgcbr, visible_cursor,
  sizeable, shell, blank, cursor, orig_emulation, font, orig_font,
  tahead, adapter, scrnmode, kbdinfo, direct_video, video_page,
  video_seg, video_ofs, bogus_adapter. (Some of these persist outside
  the SCREEN struct, in the platform directories.) Added mouse_wait and
  key_code.

- Removed all the EMALLOC stuff. Straight malloc calls were used
  elsewhere; it was undocumented outside of comments in curspriv.h; and
  there are better ways to use a substitute malloc().

- Single mouse clicks are now reportable on all platforms (not just
  double-clicks). And in general, mouse event reporting is more
  consistent across platforms.

- The mouse cursor no longer appears in full-screen mode in Win32 unless
  a nonzero mouse event mask is used.

- ALT-keypad input now works in Win32.

- In Win32, SetConsoleMode(ENABLE_WINDOW_INPUT) is not useful, and
  appears to be the source of a four-year-old bug report (hanging in
  THE) by Phil Smith.

- Removed the PDC_THREAD_BUILD stuff, which has never worked. For the
  record: PDCurses is not thread-safe. Neither is ncurses; and the
  X/Open curses spec explicitly makes it a non-requirement.

- With the internal compose key system in the X11 port, modifier keys
  were breaking out of the compose state, making it impossible to type
  accented capitals, etc. Also, Multi_key is now the default compose
  key, instead of leaving it undefined by default; and a few more combos
  are supported.

- In X11, the first reported mouse event after startup always read as a
  double-click at position 0, 0. (This bug was introduced in 2.8.)

- In X11, don't return selection start as a press event. (Shift-click on
  button 1 is still returned.)

- In X11, properly handle pasting of high-bit chars. (It was doing an
  unwanted sign extension.)

- In X11, BUTTON_MOVED was never returned, although PDC_MOUSE_MOVED was
  set.

- The fix in 2.8 for the scroll wheel in X11 wasn't very good -- it did
  report the events as scroll wheel events, but it doubled them. Here's
  a proper fix.

- Changed mouse handling in X11: Simpler translation table, with
  XCursesPasteSelection() called from XCursesButton() instead of the
  translation table; require shift with button 1 or 2 for select or
  paste when mouse events are being reported (as with ncurses), allowing
  passthrough of simple button 2 events. This fixes the previously
  unreliable button 2 behavior.

- Modifier keys are now returned on key up in X11, as in Win32. And in
  general, modifier key reporting is more consistent across platforms.

- Modifiers are not returned as keys when a mouse click has occurred
  since the key press.

- In BIOS mode (in DOS), count successive identical output bytes, and
  make only one BIOS call for all of them. This dramatically improves
  performance.

- The cursor position was not always updated correctly in BIOS mode.

- In testcurs, the way the ACS test was written, it would really only
  work with a) PDCurses (with any compiler), or b) gcc (with any
  curses). Here's a more portable implementation.

- Better reporting of mouse events in testcurs.

- Blank out buffer and num before the scanw() test in testcurs, in case
  the user just hits enter or etc.; clear the screen after resizing.

- Allow tuidemo to use the last line.

- Separate left/right modifier keys are now reported properly in Win32.
  (Everything was being reported as _R.)

- Attempts to redirect input in Win32 now cause program exit and an
  error message, instead of hanging.

- Dropped support for the Microway NDP compiler.

- Some modules renamed, rearranged.

- Fixes for errors and warnings when building with Visual C++ 2005.

- In MSVC, the panel library didn't work with the DLL.

- Complete export lists for DLLs.

- Simplified makefiles; moved common elements to .mif files; better
  optimization; strip demos when possible.

- Changed makefile targets of "pdcurses.a/lib" and "panel.a/lib" to
  $(LIBCURSES) and $(LIBPANEL). Suggestion of Doug Kaufman.

- Changed "install" target in the makefile to a double-colon rule, to
  get around a conflict with INSTALL on non-case-sensitive filesystems,
  such as Mac OS X's HFS+. Reported by Douglas Godfrey et al.

- Make PDCurses.man dependent on manext. Suggestion of Tiziano Mueller.

- Set up configure.ac so autoheader works; removed some obsolescent
  macros. Partly the suggestion of T.M.

- The X11 port now builds in the x11 directory (including the demos), as
  with other ports.

- The X11 port should now build on more 64-bit systems. Partly due to
  M.H.

- The default window title and icons for the X11 port are now "PDCurses"
  instead of "XCurses".

- Internal functions and variables made static where possible.

- Adopted a somewhat more consistent naming style: Internal functions
  with external linkage, and only those, have the prefix "PDC_";
  external variables that aren't part of the API use "pdc_"; static
  functions use "_"; and "XC_" and "xc_" prefixes are used for functions
  and variables, respectively, that are shared between both processes in
  the X11 port. Also eliminated camel casing, where possible.

- Changed the encoding for non-ASCII characters in comments and
  documentation from Latin-1 to UTF-8.

------------------------------------------------------------------------

PDCurses 2.8 - 2006-04-01
=========================

As with the previous version, you should assume that apps linked against
older dynamic versions of the library won't work with this one until
recompiled.

New features:

- Simpler, faster.

- Declarations for all supported, standard functions, per the X/Open
  Curses 4.2 spec, with the notable exception of getch() and ungetch().
  You can disable the use of the macro versions by defining NOMACROS
  before including curses.h (see xmas.c for an example). NOMACROS yields
  smaller but theoretically slower executables.

- New functions: vwprintw(), vwscanw(), vw_printw() and vw_scanw(). This
  completes the list of X/Open 4.2 functions, except for those concerned
  with attr_t and wide characters. Some (especially the terminfo/termcap
  functions) aren't yet fully fleshed out, though.

- Non-macro implementations for COLOR_PAIR(), PAIR_NUMBER(), getbkgd(),
  mvgetnstr(), mvwgetnstr(), mvhline(), mvvline(), mvwhline(), and
  mvwvline(). (The macros are still available, too.)

- newterm() works now, in a limited way -- the parameters are ignored,
  and only the first invocation will work (i.e., only one SCREEN can be
  used).

- start_color() works now -- which is to say, if you _don't_ call it,
  you'll only get monochrome output. Also, without calling it, the
  terminal's default colors will be used, where supported (currently
  only in Win32). This is equivalent to the PDC_ORIGINAL_COLORS behavior
  introduced in 2.7, except that _only_ the default colors will be used.
  (PDC_ORIGINAL_COLORS is still available, if you want to combine the
  use of specific colors and the default colors.)

- New logic for termname() and longname(): termname() always returns
  "pdcurses"; longname() returns "PDCurses for [platform] [adapter]
  [COLOR/MONO]-YxX" (adapter is only defined for DOS and OS/2). This is
  the first time these functions return _anything_ in Win32.

- New installation method for XCurses: the header files are placed in a
  subdirectory "xcurses" within the include directory, rather than being
  renamed. (But the renamed xcurses.h and xpanel.h are also installed,
  for backwards compatibility.) curspriv.h and term.h are now available,
  and existing curses-based code need no longer be edited to use
  XCurses' curses.h. And with no more need for explicit XCursesExit()
  calls (see below), your code need not be changed at all to move from
  another curses implementation to XCurses. It can be as simple as "gcc
  -I/usr/local/include/xcurses -lXCurses -oprogname progname.c".

- Combined readme.* into this HISTORY file, and incorporated the old 1.x
  (PCcurses) history.

- New functionality for the testcurs demo: ACS character display; menu
  support for PgUp, PgDn, Home and End; centered menu; and it can now
  be resized in X.

- Added modified versions of the rain and worm demos from ncurses.

Bug fixes and such:

- Big cleanup of dead and redundant code, including unneeded defines,
  ifdefs, and structure elements.

- flushinp() was not implemented for Win32.

- resetty() was not restoring LINES and COLS.

- nonl() made '\n' print a line feed without carriage return. This was
  incorrect.

- Removed bogus implementation of intrflush().

- The line-breakout optimization system, disabled by default in 2.7, is
  removed in 2.8. It simply didn't work, and never has. (The typeahead()
  function remains, for compatibility, but does nothing.)

- The declarations for the printw() and scanw() function families were
  erroneously ifdef'd.

- Safer printw() calls on platforms that support vsnprintf().

- Use the native vsscanf() in DJGPP, MinGW and Cygwin.

- ACS_BLOCK now works in X.

- Explicit calls to XCursesExit() are no longer needed.

- XCURSES is now defined automatically if not DOS, OS2 or WIN32.

- The default icon for XCurses wasn't working (had to remove the focus
  hint code to fix this). Also, the default title is now "XCurses"
  instead of "main".

- Incorrect dimensions (undercounting by two in each direction) were
  shown while resizing in X.

- Scroll wheel events were not always correctly reported in X.

- 32 bits are enough for the "long" chtype, but 64 bits were used on a
  64-bit system, wasting memory. Now conditioned on _LP64. This could be
  faster, too.

- The short, 16-bit chtype now works with XCurses.

- Corrected return value for is_linetouched(), is_wintouched(),
  can_change_color() and isendwin() (bool instead of int).

- timeout(), wtimeout(), idcok() and immedok() return void.

- pair_content() takes a short.

- Replaced incorrect usages of attr_t with chtype. attr_t is still
  typedef'd, for backwards compatibility. (It's supposed to be used for
  the WA_*-style functions, which PDCurses doesn't yet support.)

- Added const where required by the spec, and in other appropriate
  places.

- Removed PDC_usleep(). napms() is now the core delay routine.

- Fixed poll() support in napms().

- Various changes to the internal PDC_* functions -- don't depend on
  these, and don't use them unless you absolutely have to.

- Some routines accessed window structures in their variable
  declarations, _before_ checking for a NULL window pointer.

- Dropped support for the undocumented PDC_FULL_DISPLAY, wtitle(), and
  PDC_print().

- Cleaned up remaining warnings.

- Reduced unnecessary #include directives -- speeds up compilation.

- Fix for demos build in Borland/DOS -- the makefile in 2.7 didn't
  specify the memory model. Reported by Erwin Waterlander.

- Simplified the makefiles; e.g., some now build each demo in a single
  step, and Watcom no longer uses demos.lnk. Also, the demo exes are now
  stripped when possible; maximum compression used for archives built
  by the makefiles; xcurses-config removed as part of "make distclean";
  and I tweaked optimization for some platforms.

- Reverted to /usr/local/ as default installation directory for XCurses.

- Upgraded to autoconf 2.59... instantly doubling the size of the
  configure script. Ah well. Otherwise, simplified the build system.

- Dropped support for pre-ANSI compilers. (It hasn't worked since at
  least version 2.4, anyway.)

- Revised and, I hope, clarified the boilerplate and other comments.

- Simplified logging and RCS ids; added RCS ids where missing.

- Consistent formatting for all code, approximately equivalent to
  "indent -kr -i8 -bl -bli0", with adjustments for 80 columns.

------------------------------------------------------------------------

PDCurses 2.7 - 2005-12-30
=========================

INTRODUCTION:

Hello all. As of a few weeks ago, I'm the new maintainer for PDCurses.
Here's a brief summary of changes in this release. (More details are
available in the CVS log and trackers on SourceForge.)

NEW FEATURES:

- Functions: delscreen(), getattrs(), has_key(), slk_color(),
  wcolor_set(), wtimeout().

- Macros: color_set(), mvhline(), mvvline(), mvwgetnstr(), mvwhline(),
  mvwvline(), timeout(), wresize().

- Stub implementations of terminfo functions (including a term.h).

- More stubs for compatibility: filter(), getwin(), putwin(),
  noqiflush(), qiflush(), scr_dump(), scr_init(), scr_restore(),
  scr_set(), use_env(), vidattr(), vidputs().

- The terminal's default colors are used as curses' default colors when
  the environment variable "PDC_ORIGINAL_COLORS" is set to any value
  (Win32 only at the moment).

- Simplified build system.

- Replaced PDC_STATIC_BUILD with its opposite, PDC_DLL_BUILD (see .mak
  files for more info).

- Minimal implementation of color_content() -- no longer a stub.

- Added the remaining ACS defines (ACS_S3, ACS_BBSS, etc.) for
  DOS/OS2/Win; "enhanced" versions of existing ACS characters used.

- Support for scroll wheels.

- Support for Pacific C.

BUGS FIXED:

- Builds correctly (including demos) on all tested platforms (see
  below); nearly all compiler warnings have been cleaned up; the ptest
  demo is built on all platforms; "clean" targets are improved.

- The ability to build ncurses_tests has been restored (see demos dir).

- Line-breakout optimization now defaults to off (equivalent to
  "typeahead(-1)"), so output is not interrupted by keystrokes (it's
  supposed to resume on the next refresh(), which wasn't working).

- Implicit wrefresh() in wgetch() was not being invoked in nodelay mode.

- subpad() was erroneously offsetting from the origin coordinates of the
  parent pad (which are always -1,-1).

- In wborder(), whline(), and wvline(), the current (wattrset) attribute
  was being used, but not the current background (wbkgd).

- Allow Russian 'r' character ASCII 0xe0 to be returned.

- termattrs() now also returns A_UNDERLINE, A_REVERSE.

- In Win32, with large scrollback buffers set, there was an unwanted
  "scrollup" effect on startup.

- Revamped keyboard handling for Win32.

- New screen resize method for Win32.

- napms(), delay_output(), etc. now work with Cygwin.

- curs_set(0) wasn't working in Win32 in full-screen (ALT-ENTER) mode --
  the cursor stayed on.

- The A_REVERSE attribute was broken in XCurses.

- On 64-bit systems, XCurses was ignoring every other keystroke.

- Added focus hints for XCurses.

- Demos (except for tuidemo) once again have their proper titles in
  XCurses (using Xinitscr() instead of the obsolete XCursesProgramName).

- The 16-bit chtype is a working option again (by removing #define
  CHTYPE_LONG from curses.h), except in XCurses. It's not recommended;
  but if your needs are limited, it still works.

- Reset screen size in resetty() under DOS, as in Win32 and OS/2.

- Changes for cursor size under DOS.

- Automatic setting of BIOS mode for CGA under DOS now works.

- The cursor is now always updated in PDC_gotoxy(); this fixes the
  problem of missing characters in BIOS mode.

- Macros nocbreak(), cbreak(), nocrmode(), crmode(), nodelay(),
  nl() and nonl() now return OK.

- ERR and OK are now defined as -1 and 0, respectively, for
  compatibility with other curses implementations -- note that this
  change is not binary compatible; you'll have to rebuild programs that
  use shared/dynamic libraries.

- Added "const" to prototypes where appropriate.

- Miscellaneous code cleanup.

ACKNOWLEDGEMENTS:

 - Walter Briscoe
 - Jean-Pierre Demailly
 - Ruslan Fedyarov
 - Warren Gay
 - Florian Grosse-Coosmann
 - Vladimir Kokovic
 - Matt Maloy
 - K.H. Man
 - Michael Ryazanov
 - Ron Thibodeau
 - Alexandr Zamaraev

and of course, MARK HESSLING, for his over 13 years of service as the
maintainer of PDCurses. Plus, thanks to all who've reported bugs or
requested features. Apologies to anyone I've forgotten.

I've tested this version on Turbo C++ 3.0 and Borland C++ 3.1 for DOS;
DJGPP 2.X; Open Watcom 1.3 for DOS (16 and 32-bit), Windows and OS/2;
EMX 0.9d and the "newgcc" version of EMX; Borland C++ 5.5 for Windows;
recent versions of MinGW, Cygwin, LCC-Win32 and Microsoft Visual C++;
and gcc under several flavors of Linux, Mac OS X, *BSD and Solaris.

-- William McBrine

------------------------------------------------------------------------

PDCurses 2.6 - 2003-01-08
=========================

INTRODUCTION:

 This release of PDCurses includes the following changes:

BUGS FIXED:

- Allow accented characters on Win32 platform when run on non-English
  keyboards.

- Allow "special" characters like Ctrl-S, Ctrl-Q under OS/2 to be returned.

- Some bugs with halfdelay() fixed by William McBrine.

- pechochar() should now work correctly.

- redrawwin() macro in curses.h was incorrect - fixed by Alberto Ornaghi

- Don't include "special" characters like KEY_SHIFT_L to be returned in
  getnstr() family. Bug 542913

- Entering TAB in wgetnstr() no longer exceeds requested buffer size.
  Bug 489233

- Fixed bug 550066, scrollok() and pads.
  Also beep() called when buffer exceeded. Bug 562041.

- Reverse video of X11 selection reinstated. Pablo Garcia Abio??

- Right Alt modifier now works like left Alt modifier under Win32

- Add support for all libXaw replacement libraries with Scrollbar bug.
  Note that for this to work, you still have to change the libXaw
  replacement libraries to fix the bug :-(

- Don't trap signals in XCurses if calling application has ignored them.
  Change by Frank Heckenbach.

- Bug reports from Warren W. Gay:
  - Fix termattrs() to return A_REVERSE and A_BLINK on all platforms.
  - Fix definition of getsyx() and setsyx() to be consistent with
    ncurses. Bug 624424.
  - Fix definition of echo() and noecho(). Bug 625001.
  - Fix definition of keypad() and leaveok(). Bug 632653.
  - Missing panel_hidden() prototype. Bug 649320.

- Fixed bug with calling def_prog_mode(), resize_term(),
  reset_prog_mode(); the resize details were being lost.

NEW FEATURES:

- Clipboard support now available on DOS platform, but handled
  internally to the currently running process.

- New X11 resource: textCursor, allows the text cursor to be specified
  as a vertical bar, or the standard horizontal bar. Thanks to Frank
  Heckenbach for the suggestion.

NEW COMPILER SUPPORT:

- lcc-win32 now works correctly

------------------------------------------------------------------------

PDCurses 2.5 - 2001-11-26
=========================

INTRODUCTION:

 This release of PDCurses includes the following changes:

- Set BASE address for Win32 DLL

- Add KEY_SUP and KEY_SDOWN.

- Add PDC_set_line_color()

- Add blink support as bold background

- Add bold colors

- Add getbkgd() macro

- Add new PDC functions for adding underline, overline, leftline and
  rightline

- Add support for shifted keypad keys.

- Allow more keypad keys to work under Win32

- Change Win32 and OS/2 DLL name to curses.dll

- Change example resources to allow overriding from the command line

- Changes for building cleanly on OS/2

- Changes to handle building XCurses under AIX

- Check if prefresh() and pnoutrefresh() parameters are valid.

- Ensure build/install works from any directory

- Handle platforms where X11 headers do not typedef XPointer.

- Mention that Flexos is likely out-of-date.

- Pass delaytenths to XCurses_rawgetch()

- Remove boldFont

- Updates for cursor blinking and italic.

BUGS FIXED:

- Fix bug with getting Win32 clipboard contents. Added new
  PDC_freeclipboard() function.

- Fix bug with halfdelay()

- Fix bug with mouse interrupting programs that are not trapping mouse
  events under Win32.

- Fix return value from curs_set()

- Reverse the left and right pointing bars in ALT_CHARSET

NEW COMPILER SUPPORT:

- Add QNX-RTP port

------------------------------------------------------------------------

PDCurses 2.4 - 2000-01-17
=========================

INTRODUCTION:

 This release of PDCurses includes the following changes:

- full support of X11 selection handling

- removed the need for the cursos2.h file

- enabled the "shifted" key on the numeric keypad

- added native clipboard support for X11, Win32 and OS/2

- added extra functions for obtaining internal PDCurses status

- added clipboard and key modifier tests in testcurs.c

- fixes for panel library

- key modifiers pressed by themselves are now returned as keys:
  KEY_SHIFT_L KEY_SHIFT_R KEY_CONTROL_L KEY_CONTROL_R KEY_ALT_L KEY_ALT_R
  This works on Win32 and X11 ports only

- Added X11 shared library support

- Added extra slk formats supported by ncurses

- Fixed bug with resizing the terminal when slk were on.

- Changed behavior of slk_attrset(), slk_attron() slk_attroff()
  functions to work more like ncurses.

BUGS FIXED:

- some minor bug and portability fixes were included in this release

NEW FUNCTIONS:

- PDC_getclipboard() and PDC_setclipboard() for accessing the native
  clipboard (X11, Win32 and OS/2)

- PDC_set_title() for setting the title of the window (X11 and Win32
  only)

- PDC_get_input_fd() for getting the file handle of the PDCurses input

- PDC_get_key_modifiers() for getting the keyboard modifier settings at
  the time of the last (w)getch()

- Xinitscr() (only for X11 port) which allows standard X11 switches to
  be passed to the application

NEW COMPILER SUPPORT:

- MingW32 GNU compiler under Win95/NT

- Cygnus Win32 GNU compiler under Win95/NT

- Borland C++ for OS/2 1.0+

- lcc-win32 compiler under Win95/NT

ACKNOWLEDGEMENTS: (for this release)

- Georg Fuchs for various changes.
- Juan David Palomar for pointing out getnstr() was not implemented.
- William McBrine for fix to allow black/black as valid color pair.
- Peter Preus for pointing out the missing bccos2.mak file.
- Laura Michaels for a couple of bug fixes and changes required to
  support Mingw32 compiler.
- Frank Heckenbach for PDC_get_input_fd() and some portability fixes and
  the fixes for panel library.
- Matthias Burian for the lcc-win32 compiler support.

------------------------------------------------------------------------

PDCurses 2.3 - 1998-07-09
=========================

INTRODUCTION:

This release of PDCurses includes the following changes:

- added more System V R4 functions

- added Win32 port

- the X11 port is now fully functional

- the MS Visual C++ Win32 port now includes a DLL

- both the X11 and Win32 ports support the mouse

- the slk..() functions are now functional

- support for scrollbars under X11 are experimental at this stage

- long chtype extended to non-Unix ports

The name of the statically built library is pdcurses.lib (or
pdcurses.a). The name of the DLL import library (where applicable) is
curses.lib.

BUGS FIXED:

- some minor bugs were corrected in this release

NEW FUNCTIONS:

- slk..() functions

NEW COMPILER SUPPORT:

- MS Visual C++ under Win95/NT

- Watcom C++ under OS/2, Win32 and DOS

- two EMX ports have been provided:
  - OS/2 only using OS/2 APIs
  - OS/2 and DOS using EMX video support routines

EXTRA OPTIONS:

PDCurses recognizes two environment variables which determines the
initialization and finalization behavior.  These environment variables
do not apply to the X11 port.

PDC_PRESERVE_SCREEN -
If this environment variable is set, PDCurses will not clear the screen
to the default white on black on startup.  This allows you to overlay
a window over the top of the existing screen background.

PDC_RESTORE_SCREEN -
If this environment variable is set, PDCurses will take a copy of the
contents of the screen at the time that PDCurses is started; initscr(),
and when endwin() is called, the screen will be restored.


ACKNOWLEDGEMENTS: (for this release)

- Chris Szurgot for original Win32 port.
- Gurusamy Sarathy for some updates to the Win32 port.
- Kim Huron for the slk..() functions.
- Florian Grosse Coosmann for some bug fixes.
- Esa Peuha for reducing compiler warnings.
- Augustin Martin Domingo for patches to X11 port to enable accented
  characters.

------------------------------------------------------------------------

PDCurses 2.2 - 1995-02-12
=========================

INTRODUCTION:

 This release of PDCurses has includes a number of major changes:

- The portable library functions are now grouped together into single
  files with the same arrangement as System V R4 curses.

- A panels library has been included. This panels library was written by
  Warren Tucker.

- Quite a few more functions have been supplied by Wade Schauer and
  incorporated into release 2.2. Wade also supplied the support for the
  Microway NDP C/C++ 32 bit DOS compiler.

- The curses datatype has been changed from an unsigned int to a long.
  This allows more attributes to be stored as well as increasing the
  number of color-pairs from 32 to 64.

- Xwindows port (experimental at the moment).

BUGS FIXED:

- mvwin() checked the wrong coordinates

- removed DESQview shadow memory buffer checking bug in curses.h in
  \#define for wstandout()

- lots of others I can't remember

NEW FUNCTIONS:

- Too many to mention. See intro.man for a complete list of the
  functions PDCurses now supports.

COMPILER SUPPORT:

- DJGPP 1.12 is now supported. The run-time error that caused programs
  to crash has been removed.

- emx 0.9a is supported. A program compiled for OS/2 should also work
  under DOS if you use the VID=EMX switch when compiling. See the
  makefile for details.

- The Microway NDP C/C++ DOS compiler is now supported. Thanks to Wade
  Schauer for this port.

- The Watcom C++ 10.0 DOS compiler is now supported. Thanks to Pieter
  Kunst for this port.

- The library now has many functions grouped together to reduce the size
  of the library and to improve the speed of compilation.

- The "names" of a couple of the compilers in the makefile has changed;
  CSET2 is now ICC and GO32 is now GCC.

EXTRA OPTIONS:

 One difference between the behavior of PDCurses and Unix curses is the
 attributes that are displayed when a character is cleared. Under Unix
 curses, no attributes are displayed, so the result is always black.
 Under PDCurses, these functions clear with the current attributes in
 effect at the time. With the introduction of the bkgd functions, by
 default, PDCurses clears using the value set by (w)bkgd(). To have
 PDCurses behave the same way as it did before release 2.2, compile with
 -DPDCURSES_WCLR

ACKNOWLEDGEMENTS: (for this release)

 Pieter Kunst, David Nugent, Warren Tucker, Darin Haugen, Stefan Strack,
 Wade Schauer and others who either alerted me to bugs or supplied
 fixes.

------------------------------------------------------------------------

PDCurses 2.1 - 1993-06-20
=========================

INTRODUCTION:

 The current code contains bug fixes for the DOS and OS/2 releases and
 also includes an alpha release for Unix. The Unix release uses another
 public domain package (mytinfo) to handle the low-level screen writes.
 mytinfo was posted to comp.sources.unix (or misc) in December 1992 or
 January 1993. Unless you are a glutton for punishment I would recommend
 you avoid the Unix port at this stage.

 The other major addition to PDCurses is the support for DJGPP (the DOS
 port of GNU C++). Thanks to David Nugent <davidn@csource.oz.au>.

 Other additions are copywin() function, function debugging support and
 getting the small and medium memory models to work. The testcurs.c demo
 program has also been changed significantly and a new demo program,
 tuidemo, has been added.

 Some people have suggested including information on where to get dmake
 from. oak.oakland.edu in /pub/msdos/c

OTHER NOTES:

 Under DOS, by default, screen writes to a CGA monitor are done via the
 video BIOS rather than by direct video memory writes. This is due to
 the CGA "snow" problem. If you have a CGA monitor and do not suffer
 from snow, you can compile private\_queryad.c with CGA_DIRECT defined.
 This will then use cause PDCurses to write directly to the CGA video
 memory.

 Function debugging: Firstly to get function debugging, you have to
 compile the library with OPT=N in the makefile. This also turns on
 compiler debugging. You can control when you want PDCurses to write to
 the debug file (called trace in the current directory) by using the
 functions traceon() and traceoff() in your program.

 Microsoft C 6.00 Users note:
 ----------------------------

 With the addition of several new functions, using dmake to compile
 PDCurses now causes the compiler to run "out of heap space in pass 2".
 Using the 6.00AX version (DOS-Extended) to compile PDCurses fixes this
 problem; hence the -EM switch.

 Functional changes
 ------------------

 Added OS/2 DLL support.

 A few curses functions have been fixed to exhibit their correct
 behavior and make them more functionally portable with System V
 curses. The functions that have changed are overlay(), overwrite() and
 typeahead.

 overlay() and overwrite()

 Both of theses functions in PDCurses 2.0 allowed for one window to be
 effectively placed on top of another, and the characters in the first
 window were overlaid or overwritten starting at 0,0 in both windows.
 This behavior of these functions was not correct. These functions only
 operate on windows that physically overlap with respect to the
 displayed screen. To achieve the same functionality as before, use the
 new function copywin(). See the manual page for further details.

 typeahead()

 This function in PDCurses 2.0 effectively checked to see if there were
 any characters remaining in the keyboard buffer. This is not the
 behavior exhibited by System V curses. This function is intended
 purely to set a flag so that curses can check while updating the
 physical screen if any keyboard input is pending. To achieve the same
 effect with typeahead() under PDCurses 2.1 the following code should be
 used.

 In place of...

       while(!typeahead(stdin))
        {
          /* do something until any key is pressed... */
        }

 use...

       /* getch() to return ERR if no key pending */
       nodelay(stdscr,TRUE);
       while(getch() == (ERR))
        {
          /* do something until any key is pressed... */
        }


ACKNOWLEDGEMENTS: (in no particular order)

 Jason Shumate, Pieter Kunst, David Nugent, Andreas Otte, Pasi
 Hamalainen, James McLennan, Duane Paulson, Ib Hojme

 Apologies to anyone I may have left out.

------------------------------------------------------------------------

PDCurses 2.0 - 1992-11-23
=========================

INTRODUCTION:

 Well, here it finally is; PDCurses v2.0.

 PDCurses v2.0 is an almost total rewrite of PCcurses 1.4 done by John
 'Frotz' Fa'atuai, the previous maintainer. It adds support for OS/2 as
 well as DOS.

 This version has been tested with Microsoft C v6.0, QuickC v2.0 and
 Borland C++ 2.0 under DOS and Microsoft C v6.0 and TopSpeed c v3.02
 under OS/2 2.0. Also the library has been compiled successfully with
 emx 0.8e, C Set/2 and Watcom 9. Most testing was done with the large
 memory model, where applicable. The large memory model is probably the
 best model to use.

 The amount of testing has not been as extensive as I would have liked,
 but demands on releasing a product have outweighed the product's
 quality. Nothing new with that !! Hopefully with wider circulation,
 more bugs will be fixed more quickly.

 I have included just 1 makefile which is suitable for dmake 3.8 for
 both DOS and OS/2. The makefile does not rely on customization of the
 dmake.ini file.

 If you discover bugs, and especially if you have fixes, please let me
 know ASAP.

 The source to the library is distributed as a zip file made with zip
 1.9. You will need Info-ZIP unzip 5.0 to unzip. Follow the directions
 below to compile the library.

DIRECTIONS:

 1. Create a new directory in which to unzip pdcurs20.zip. This will
    create a curses directory and a number of subdirectories containing
    source code for the library and utilities and the documentation.

 2. Make changes to the makefile where necessary:
    Change the MODEL or model macro to the appropriate value (if it
    applies to your compiler). Use model for Borland compilers.

    Change any paths in the defined macros to be suitable for your
    compiler.

 3. Invoke DMAKE [-e environment_options] [target]

    where environment_options are:

        OS (host operating system)
        COMP (compiler)
        OPT (optimized version or debug version) - optional. default Y
        TOS (target operating system) - optional. default OS

    see the makefile for valid combinations

    targets: all, demos, lcursesd.lib, manual...

    NB. dmake is case sensitive with targets, so those environments that
    use an upper case model value (eg MSC) MUST specify the library
    target as for eg. Lcursesd.lib

    The makefile is by default set up for Borland C++. The use of -e
    environment_options override these defaults. If you prefer, you can
    just change the defaults in the makefile and invoke it without the
    -e switch.

OTHER NOTES:

 The documentation for the library is built into each source file, a
 couple of specific doc files and the header files. A program is
 supplied (manext) to build the manual. This program gets compiled when
 you build the documentation.

 To generate the library response file correctly, I had to write a quick
 and dirty program (buildlrf) to achieve this. Originally the makefiles
 just had statements like: "echo -+$(OBJ)\$* & >> $(LRF)" which appended
 a suitable line to the response file. Unfortunately under some
 combinations of makefiles and command processors (eg. nmake and 4DOS)
 the & would get treated as stderr and the echo command would fail.

 The original source for PDCurses that I received from the previous
 maintainer contained support for the FLEXOS operating system. Not
 having access to it, I could not test the changes I made so its support
 has fallen by the wayside. If you really need to have PDCurses running
 under FLEXOS, contact me and I will see what can be arranged.

 Under DOS, by default, screen writes to a CGA monitor are done via the
 video BIOS rather than by direct video memory writes. This is due to
 the CGA "snow" problem. If you have a CGA monitor and do not suffer
 from snow, you can compile private\_queryad.c with CGA_DIRECT defined.
 This will then use cause PDCurses to write directly to the CGA video
 memory.

 Added System V color support.

COMPILER-SPECIFIC NOTES:

 Microsoft C
 -----------

 It is possible with MSC 6.0 to build the OS/2 libraries and demo
 programs from within DOS. This is the only case where it is possible to
 specify the value of TOS on the command line to be OS2 and the value of
 OS be DOS.

 C Set/2
 -------

 I have only tested the library using the migration libraries. I doubt
 that the demo programs will work without them.

 emx
 ---

 Testing has been done with 0.8e of emx together with the 16_to_32
 libraries. The emx\lib directory should include the vio32.lib and
 kbd32.lib libraries from the 16_to_32 package.

BUGS and UNFINISHED BUSINESS:

- PDC_set_ctrl_break() function does not work under OS/2.

- win_print() and PDC_print() do not work under OS/2.

- The file todo.man in the doc directory also lists those functions of
  System V 3.2 curses not yet implemented. Any volunteers?

ACKNOWLEDGEMENTS:

- John 'Frotz' Fa'atuai, the previous maintainer for providing an
  excellent base for further development.
- John Burnell <johnb@kea.am.dsir.govt.nz>, for the OS/2 port.
- John Steele, Jason (finally NOT a John) Shumate....
  for various fixes and suggestions.
- Eberhardt Mattes (author of emx) for allowing code based on his
  C library to be included with PDCurses.
- Several others for their support, moral and actual.

-- Mark Hessling

------------------------------------------------------------------------

PDCurses 2.0Beta - 1991-12-21
=============================

Changed back from short to int. (int is the correct size for the default
platform. Short might be too short on some platforms. This is more
portable. I, also, made this mistake.)

Many functions are now macros.  If you want the real thing, #undef the
macro. (X/Open requirement.)

Merged many sources into current release.

Added many X/Open routines (not quite all yet).

Added internal documentation to all routines.

Added a HISTORY file to the environment.

Added a CONTRIB file to the environment.

------------------------------------------------------------------------

PDCurses 1.5Beta - 1990-07-14
=============================

Added many levels of compiler support. Added mixed prototypes for all
"internal" routines. Removed all assembly language.  Added EGA/VGA
support.  Converted all #ifdef to #if in all modules except CURSES.H and
CURSPRIV.H. Always include ASSERT.H.  Added support for an external
malloc(), calloc() and free(). Added support for FAST_VIDEO
(direct-memory writes). Added various memory model support (for
FAST_VIDEO). Added much of the December 1988 X/Open Curses
specification.

-- John 'Frotz' Fa'atuai

------------------------------------------------------------------------

PCcurses 1.4 - 1990-01-14
=========================

  In PCcurses v.1.4, both portability improvements and bugfixes have
been made. The files have been changed to allow lint-free compilation
with Microsoft C v.5.1, and with Turbo C v.2.0. The source should still
compile without problems on older compilers, although this has not been
verified.

  The makefiles have been changed to suit both the public release and
the author, who maintains a special kind of libraries for himself. In
the case of Microsoft C, changes were done in the makefile to lower the
warning level to 2 (was 3). This was to avoid ANSI warnings which are
abundant because PCcurses does not attempt to follow strict ANSI C
standard.

  BUG FIXES FROM V.1.3 TO V.1.4:

  !!!IMPORTANT CHANGE!!!

  The definitions for OK and ERR in curses.h were exchanged. This was
done to be more consistent with UNIX versions. Also, it permits
functions like newwin() and subwin() to return 0 (=NULL) when they fail
due to memory shortage. This incompatibility with UNIX curses was
pointed out by Fred C. Smith. If you have tested success/failure by
comparisons to anything other than ERR and OK, your applications will
need to be be changed on that point. Sorry... but presumably most of you
used the symbolic constants?

  (END OF IMPORTANT CHANGE)

  Fred also pointed out a bug in the file update.c. The bug caused the
first character printed after 'unauthorized' screen changes (like during
a shell escape, for example) to be placed at the wrong screen position.
This happened even if the normal precautions (clear / touch / refresh)
were taken. The problem has now been fixed.

  PCcurses is currently also being used on a 68000 system with
hard-coded ESCape sequences for ANSI terminals. However, ints used by
the 68000 C compiler are 32 bits. Therefore ints have been turned into
shorts wherever possible in the code (otherwise all window structures
occupy twice as much space as required on the 68000). This does not
affect PC versions since normally both ints and shorts are 16 bits for
PC C compilers.

  At some places in the source code there are references made to the
68000 version. There are also a makefile, a curses68.c file, and a
curses68.cmd file. These are for making, low-level I/O, and linking
commands when building the 68000 version. These files are probably
useful to no-one but the author, since it is very specific for its
special hardware environment. Still in an effort to keep all
curses-related sources in one place they are included. Note however that
PCcurses will not officially support a non-PC environment.

  The file cursesio.c, which was included in the package at revision
level 1.2, and which was to be an alternative to the cursesio.asm file,
has been verified to behave incorrectly in the function _curseskeytst().
The problem was that the value of 'cflag' does not contain the proper
data for the test that is attempted. Furthermore, neither Turbo C or
Microsoft C allows any way to return the data that is needed, and
consequently you should not use cursesio.c. The best solution is to
simply use the ASM version. In v.1.2 and v.1.3, the user could edit the
makefile to select which version he wanted to use. The makefiles in
v.1.4 have removed this possibility forcing the use of the ASM file, and
cursesio.c has been dropped from the distribution.

  A bug in the wgetstr() function caused PCcurses to echo characters
when reading a keyboard string, even if the echo had been turned off.
Thanks to Per Foreby at Lund University, Sweden, for this. Per also
reported bugs concerning the handling of characters with bit 8 set.
Their ASCII code were considered as lower than 32, so they were erased
etc. like control characters, i.e. erasing two character positions. The
control character test was changed to cope with this.

  The overlay() and overwrite() functions were changed so that the
overlaying window is positioned at its 'own' coordinates inside the
underlying window (it used to be at the underlying window's [0,0]
position). There is some controversy about this - the documentation for
different curses versions say different things. I think the choice made
is the most reasonable.

  The border() and wborder() functions were changed to actually draw a
border, since this seems to be the correct behavior of these functions.
They used to just set the border characters to be used by box(). These
functions are not present in standard BSD UNIX curses.

  The subwin() function previously did not allow the subwindow to be as
big as the original window in which it was created. This has now been
fixed. There was also the problem that the default size (set by
specifying numlines or numcols (or both) as 0 made the resulting actual
size 1 line/column too small.

  There were a few spelling errors in function names, both in the
function declarations and in curses.h. This was reported by Carlos
Amaral at INESC in Portugal. Thanks! There was also an unnecessary (but
harmless) parameter in a function call at one place.

------------------------------------------------------------------------

PCcurses 1.3 - 1988-10-05
=========================

  The file 'border.c' is now included. It allows you to explicitly
specify what characters should be used as box borders when the box()
functions are called. If the new border characters are non-0, they
override the border characters specified in the box() call. In my
understanding, this functionality is required for AT&T UNIX sV.3
compatibility. Thanks for this goes to Tony L. Hansen
(hansen@pegasus.UUCP) for posting an article about it on Usenet
(newsgroup comp.unix.questions; his posting was not related at all to
PCcurses).

  The only other difference between v.1.2 and v.1.3 is that the latter
has been changed to avoid warning diagnostics if the source files are
compiled with warning switches on (for Microsoft this means '-W3', for
Turbo C it means '-w -w-pro'). Of these, the Turbo C warning check is
clearly to be used rather than Microsoft, even if neither of them comes
even close to a real UNIX 'lint'. Some of the warnings in fact indicated
real bugs, mostly functions that did not return correct return values or
types.

  The makefiles for both MSC and TRC have been modified to produce
warning messages as part of normal compilation.

------------------------------------------------------------------------

PCcurses 1.2 - 1988-10-02
=========================

  The changes from v.1.1 to v.1.2 are minor. The biggest change is that
there was a bug related to limiting the cursor movement if the
application tried to move it outside the screen (something that should
not be done anyway). Such erroneous application behavior is now handled
appropriately.

  All modules have been changed to have a revision string in them, which
makes it easier to determine what version is linked into a program (or
what library version you have).

  There is now a 'cursesio.c' file. That file does the same as
'cursesio.asm' (i.e. it provides the interface to the lower-level system
I/O routines). It is written in C and thus it is (possibly) more
portable than the assembler version (but still not so portable since it
uses 8086 INT XX calls directly). When one creates new curses libraries,
one chooses whether to use the assembler or the C version of cursesio.
The choice is made by commenting out the appropriate dependencies for
cursesio.obj, near the end of the makefiles.

  There is now a 'setmode.c' file. That file contains functions that
save and restore terminal modes. They do it into other variables than do
savetty() and resetty(), so one should probably use either
savetty()/resetty() or the new functions only - and not mix the both
ways unless one really knows what one does.

  Diff lists vs v.1.0 are no longer included in the distribution. The
make utility still is. PCcurses v.1.2 still compiles with Microsoft C
v.4.0, and with Borland Turbo C v.1.0. There is as far as I know no
reason to believe that it does not compile under Microsoft C v.3.0 and
5.x, or Turbo C v.1.5, but this has not been tested.

  There are two makefiles included, one for Microsoft C, one for Turbo
C. They are both copies of my personal makefiles, and as such they
reflect the directory structure on my own computer. This will have to be
changed before you run make. Check $(INCDIR) and $(LIBDIR) in
particular, and make the choice of ASM or C cursesio version as
mentioned above (the distribution version uses the C version of
cursesio).

  The manual file (curses.man) has been changed at appropriate places.

  I would like to thank the following persons for their help:

       Brandon S. Allbery (alberry@ncoast.UUCP)
               for running comp.binaries.ibm.pc (at that time)
               and comp.source.misc.

       Steve Balogh (Steve@cit5.cit.oz.AU)
               for writing a set of manual pages and posting
               them to the net.

       Torbjorn Lindh
               for finding bugs and suggesting raw
               character output routines.

       Nathan Glasser (nathan@eddie.mit.edu)
               for finding and reporting bugs.

       Ingvar Olafsson (...enea!hafro!ingvar)
               for finding and reporting bugs.

       Eric Rosco (...enea!ipmoea!ericr)
               for finding and reporting bugs.

       Steve Creps (creps@silver.bacs.indiana.edu)
               for doing a lot of work - among others
               posting bug fixes to the net, and writing
               the new cursesio.c module.

       N. Dean Pentcheff (dean@violet.berkeley.edu)
               for finding bugs and rewriting cursesio.asm
               for Turbo 'C' 1.5.

  Finally, Jeff Dean (parcvax,hplabs}!cdp!jeff)
                    (jeff@ads.arpa)
       has had a shareware version of curses deliverable since
       about half a year before I released PCcurses 1.0 on Use-
       Net. He is very concerned about confusion between the two
       packages, and therefore any references on the network
       should make clear whether they reference Dean's PCcurses
       or Larsson's PCcurses.

------------------------------------------------------------------------

PCcurses 1.1 - 1988-03-06
=========================

  The changes from v.1.0 to v.1.1 are minor. There are a few bug fixes,
and new (non-portable) functions for verbatim IBM character font display
have been added (in charadd.c and charins.c). The manual file
(curses.man) has been changed at appropriate places.

  In the file v10tov11.dif there are listings of the differences between
version 1.0 and 1.1. The diff listings are in UNIX diff(1) format.

  Version 1.1 compiles with Turbo C v.1.0, as well as Microsoft C v.3.0
and v.4.0. On the release disk there is a make.exe utility which is very
similar to UNIX make (If the package was mailed to you, the make utility
will be in uuencoded format - in make.uu - and must be uudecoded first).
It is much more powerful than Microsoft's different MAKEs; the latter
ones will NOT generate libraries properly if used with the PCcurses
makefiles.

  There are three makefiles:

       makefile      generic MSC 3.0 makefile
       makefile.ms      MSC 4.0 makefile
       makefile.tc      Turbo C 1.0 makefile

  To make a library with for example Turbo C, make directories to hold
.H and .LIB files (these directories are the 'standard places'), edit
makefile.tc for this, and type

       make -f makefile.tc all

and libraries for all memory models will be created in the .LIB
directory, while the include files will end up in the .H directory. Also
read what is said about installation below!

------------------------------------------------------------------------

PCcurses 1.0 - 1987-08-24
=========================

  This is the release notes for the PCcurses v.1.0 cursor/window control
package. PCcurses offers the functionality of UNIX curses, plus some
extras. Normally it should be possible to port curses-based programs
from UNIX curses to PCcurses on the IBM PC without changes. PCcurses is
a port/ rewrite of Pavel Curtis' public domain 'ncurses' package. All
the code has been re-written - it is not just an edit of ncurses (or
UNIX curses). I mention this to clarify any copyright violation claims.
The data structures and ideas are very similar to ncurses. As for UNIX
curses, I have not even seen any sources for it.

  For an introduction to the use of 'curses' and its derivatives, you
should read 'Screen Updating and Cursor Movement Optimization: A Library
Package' by Kenneth C. R. C. Arnold, which describes the original
Berkeley UNIX version of curses. It is available as part of the UNIX
manuals. The other source of information is 'The Ncurses Reference
Manual' by Pavel Curtis. The latter is part of Curtis' ncurses package.

  The only other documentation provided is a 'man' page which describes
all the included functions in a very terse way. In the sources, each
function is preceded by a rather thorough description of what the
function does. I didn't have time to write a nice manual/tutorial -
sorry.

  PCcurses is released as a number of source files, a man page, and a
make file. A uuencoded copy of a 'make' utility, and a manpage for the
'make' is also provided to make it easier to put together PCcurses
libraries. Even if you are not interested in PCcurses, it may be
worthwhile to grab the make.

  The makefile assumes the presence of the Microsoft C compiler (3.0 or
4.0), Microsoft MASM and LIB, plus some MS-DOS utilities. The reason for
supplying MAKE.EXE is that the Microsoft 'MAKE:s' are much inferior to a
real UNIX make. The supplied make is a port of a public domain make,
published on Usenet. It is almost completely compatible with UNIX make.
When generating the curses libraries, the makefile will direct make to
do some directory creating and file copying, and then re-invoke itself
with new targets. The workings of the makefile are not absolutely
crystal clear at first sight... just start it and see what it does.

  For portability, the curses libraries depend on one assembler file for
access to the BIOS routines. There is no support for the EGA, but both
CGA, MGA, and the HGA can be used. The libraries are originally for
Microsoft C, but all C modules should be portable right away. In the
assembler file, segment names probably need to be changed, and possibly
the parameter passing scheme. I think Turbo C will work right away - as
far as I understand, all its conventions are compatible with Microsoft
C.

  There are some parts left out between ncurses and PCcurses. One is the
support for multiple terminals - not very interesting on a PC anyway.
Because we KNOW what terminal we have, there is no need for a termcap or
terminfo library. PCcurses also has some things that neither curses nor
ncurses have. Compared to the original UNIX curses, PCcurses has lots of
extras.

  The BIOS routines are used directly, which gives fast screen updates.
PCcurses does not do direct writes to screen RAM - in my opinion it is
a bit ugly to rely that much on hardware compatibility. Anyone could fix
that, of course...

  One of the more serious problems with PCcurses is the way in which
normal, cbreak, and raw input modes are done. All those details are in
the 'charget' module - I do raw I/O via the BIOS, and perform any
buffering myself. If an application program uses PCcurses, it should do
ALL its I/O via PCcurses calls, otherwise the mix of normal and
PCcurses I/O may mess up the display. I think my code is reasonable...
comments are welcome, provided you express them nicely...

  To install, copy all files to a work directory, edit 'makefile' to
define the standard include and library file directory names of your
choice (these directories must exist already, and their path names must
be relative to the root directory, not to the current one). You must
also run uudecode on make.uu, to generate MAKE.EXE. You can do that on
your PC, if you have uudecode there, otherwise you can do it under UNIX
and do a binary transfer to the PC. When you have MAKE.EXE in your work
directory (or in your /bin directory), type make.

  Make will now create 4 sub-directories (one for each memory model),
copy some assembler include files into them, copy two include files to
your include directory, CHDIR to each sub-directory and re-invoke itself
with other make targets to compile and assemble all the source files
into the appropriate directories. Then the library manager is run to
create the library files in your desired library directory. Presto!

  If you only want to generate a library for one memory model, type
'make small', 'make large', etc. The name of the memory model must be in
lower case, like in the makefile.

  I think the package is fairly well debugged - but then again, that's
what I always think. It was completed in May-87, and no problems found
yet. Now it's your turn... Comments, suggestions and bug reports and
fixes (no flames please) to

-- Bjorn Larsson
