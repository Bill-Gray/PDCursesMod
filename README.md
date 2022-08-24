Welcome to PDCursesMod!
=======================

Public Domain Curses (Modified), aka PDCursesMod, is an implementation of X/Open curses for multiple platforms. As the name suggests,  it is a modified version of the ['official' PDCurses project](https://www.github.com/wmcbrine/PDCurses),  adding a variety of improvements, such as Windows GUI, VT-like console,  Linux/*BSD framebuffer,  DOSVGA,  and Plan 9 "flavors" of Curses.  It expands many of the capabilities of the flavors inherited from PDCurses (SDL1,  SDL2,  X11, Win32 console, OS/2, and DOS).  Some details and screen shots are at

https://www.projectpluto.com/win32a.htm

Much of this was done during a long period in which the 'official' PDCurses  was inactive.  Since then,  that version has again been intermittently active;  most improvements made there have been taken into this project,  and a few improvements from here have gone into PDCurses. Most programs that compile and run with PDCurses or ncurses will also compile with PDCursesMod,  and vice versa, as long as you stick to the (substantial) subset of common functionality. However,  the fork appears to be permanent.

Legal Stuff
-----------

The core package is in the public domain, but small portions of PDCursesMod are subject to copyright under various licenses.  Each directory contains a README file, with a section titled "Distribution Status" which describes the status of the files in that directory.

If you use PDCursesMod in an application, an acknowledgement would be appreciated, but is not mandatory. If you make corrections or enhancements to PDCursesMod, please forward them to the current maintainer for the benefit of other users.

This software is provided AS IS with NO WARRANTY whatsoever.

Ports
-----

PDCurses has been ported to DOS, OS/2, Win32, X11 and SDL1 and SDL2. PDCursesMod adds DOSVGA,  Windows GUI,  Plan 9,  Linux/*BSD framebuffer,  and VT-like console ports. A directory containing the port-specific source files exists for each of these platforms.

Build instructions are in the README file for each platform:

-  [DOS](dos/README.md) for use on DOS
-  [DOSVGA](dosvga/README.md) for use on DOS in graphical modes
-  [fb](fb/README.md) for use on the Linux/*BSD framebuffer
-  [OS/2](os2/README.md) for use on OS/2
-  [Plan9](plan9/README.md) for use on Plan9 (APE)
-  [SDL 1.x](sdl1/README.md) for use as separate SDL version 1 window
-  [SDL 2.x](sdl2/README.md) for use as separate SDL version 2 window
-  [wincon](wincon/README.md) (formerly win32) for use on Windows Console
-  [WinGUI](wingui/README.md) for use on Windows Graphics Mode
-  [X11](x11/README.md) (also called XCurses) for use as separate X11 window
-  [VT](vt/README.md) for use on terminal

Distribution Status
-------------------

All files in this directory except configure, config.guess and config.sub are released to the Public Domain. config.guess and config.sub are under the GPL; configure is under a free license described within it.

Build Status
-------------------

PDCursesMod is automatically built in multiple environments:

[![Build](https://github.com/Bill-Gray/PDCursesMod/actions/workflows/github_actions_build.yml/badge.svg)](https://github.com/Bill-Gray/PDCursesMod/actions/workflows/github_actions_build.yml), [![Build status Travis-CI](https://api.travis-ci.com/Bill-Gray/PDCursesMod.svg?branch=master)](https://travis-ci.com/Bill-Gray/PDCursesMod)
, also providing artifacts: [![Build status Appveyor](https://ci.appveyor.com/api/projects/status/github/Bill-Gray/PDCursesMod?branch=master&svg=true)](https://ci.appveyor.com/project/Bill-Gray/PDCursesMod)


Maintainer
----------

Bill Gray
p&#x202e;&ocirc;&#xe7;.&ouml;tulp&#x165;c&eacute;j&ocirc;&#x159;p&#x40;ot&uacute;l&#x202c;m
