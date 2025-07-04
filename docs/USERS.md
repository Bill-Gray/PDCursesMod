PDCurses User's Guide
=====================

Curses Overview
---------------

The X/Open Curses Interface Definition describes a set of C-Language
functions that provide screen-handling and updating, which are
collectively known as the curses library.

The curses library permits manipulation of data structures called
windows which may be thought of as two-dimensional arrays of
characters representing all or part of a terminal's screen.  The
windows are manipulated using a procedural interface described
[elsewhere].  The curses package maintains a record of what characters
are on the screen.  At the most basic level, manipulation is done with
the routines move() and addch() which are used to "move" the curses
around and add characters to the default window, stdscr, which
represents the whole screen.

An application may use these routines to add data to the window in any
convenient order.  Once all data have been added, the routine
refresh() is called.  The package then determines what changes have
been made which affect the screen.  The screen contents are then
changed to reflect those characters now in the window, using a
sequence of operations optimized for the type of terminal in use.

At a higher level routines combining the actions of move() and addch()
are defined, as are routines to add whole strings and to perform
format conversions in the manner of printf().

Interfaces are also defined to erase the entire window and to specify
the attributes of individual characters in the window.  Attributes
such as inverse video, underline and blink can be used on a
per-character basis.

New windows can be created by allowing the application to build
several images of the screen and display the appropriate one very
quickly.  New windows are created using the routine newwin().  For
each routine that manipulates the default window, stdscr, there is a
corresponding routine prefixed with w to manipulate the contents of a
specified window; for example, move() and wmove().  In fact, move(...)
is functionally equivalent to wmove( stdscr, ...).  This is similar to
the interface offered by printf(...) and fprintf(stdout, ...).

Windows do not have to correspond to the entire screen.  It is
possible to create smaller windows, and also to indicate that the
window is only partially visible on the screen.  Furthermore, large
windows or pads, which are bigger than the actual screen size, may be
created.

Interfaces are also defined to allow input character manipulation and
to disable and enable many input attributes: character echo, single
character input with or without signal processing (cbreak or raw
modes), carriage returns mapping to newlines, screen scrolling, etc.


Data Types and the \<curses.h\> Header
--------------------------------------

The data types supported by curses are described in this section.

As the library supports a procedural interface to the data types, actual
structure contents are not described.  All curses data are manipulated
using the routines provided.


### The \<curses.h\> Header

The \<curses.h\> header defines various constants and declares the data
types that are available to the application.


### Data Types

The following data types are declared:

   Type         | Description
   :------------|:------------------------------------
   WINDOW *     | pointer to screen representation
   SCREEN *     | pointer to terminal descriptor
   bool         | boolean data type
   chtype       | representation of a character in a window
   cchar_t      | the wide-character equivalent of chtype
   attr_t       | for WA_-style attributes

The actual WINDOW and SCREEN objects used to store information are
created by the corresponding routines and a pointer to them is provided.
All manipulation is through that pointer.


### Variables

The following variables are defined:

   Variable     | Description
   :------------|:------------------------------------
   LINES        | number of lines on terminal screen
   COLS         | number of columns on terminal screen
   stdscr       | pointer to the default screen window
   curscr       | pointer to the current screen image
   SP           | pointer to the current SCREEN struct
   Mouse_status | status of the mouse
   COLORS       | number of colors available
   COLOR_PAIRS  | number of color pairs available
   TABSIZE      | size of one TAB block
   acs_map[]    | alternate character set map
   ttytype[]    | terminal name/description


### Constants

The following constants are defined:

#### General

   Constant     | Description
   :------------|:------------------------------------
   FALSE        | boolean false value
   TRUE         | boolean true value
   NULL         | zero pointer value
   ERR          | value returned on error condition
   OK           | value returned on successful completion

#### Video Attributes

Normally, attributes are a property of the character.

For chtype:

   Attribute    | Description
   :------------|:------------------------------------
   A_ALTCHARSET | use the alternate character set
   A_BLINK      | bright background or blinking
   A_BOLD       | bright foreground or bold
   A_DIM        | half bright
   A_INVIS      | invisible -- no effect in PDCurses
   A_ITALIC     | italic
   A_LEFT       | line along the left edge
   A_PROTECT    | protected -- no effect in PDCurses
   A_REVERSE    | reverse video
   A_RIGHT      | line along the right edge
   A_STANDOUT   | terminal's best highlighting mode
   A_STRIKEOUT  | strikeout
   A_UNDERLINE  | underline
   A_ATTRIBUTES | bit-mask to extract attributes
   A_CHARTEXT   | bit-mask to extract a character
   A_COLOR      | bit-mask to extract a color-pair

Not all attributes will work on all terminals. A_ITALIC is not standard,
but is shared with ncurses.  A_STRIKEOUT is a PDCursesMod extension.

For attr_t:

   Attribute    | Description
   :------------|:-------------------------------------
   WA_ALTCHARSET| same as A_ALTCHARSET
   WA_BLINK     | same as A_BLINK
   WA_BOLD      | same as A_BOLD
   WA_DIM       | same as A_DIM
   WA_INVIS     | same as A_INVIS
   WA_ITALIC    | same as A_ITALIC
   WA_LEFT      | same as A_LEFT
   WA_PROTECT   | same as A_PROTECT
   WA_REVERSE   | same as A_REVERSE
   WA_RIGHT     | same as A_RIGHT
   WA_STANDOUT  | same as A_STANDOUT
   WA_STRIKEOUT | same as A_STRIKEOUT
   WA_UNDERLINE | same as A_UNDERLINE

Note that on PDCurses*,  ncurses,  and most other implementations,  the
WA_* macros are identical to the A_* macros.  This is not guaranteed,  and
some implementations (Solaris xpg4 curses,  for example) do use different
values for WA_* and A_*.  So care is advised.

The following are also defined, for compatibility, but currently have
no effect in PDCursesMod (or any known implementation of Curses):
A_HORIZONTAL, A_VERTICAL, A_LOW and their WA_* equivalents.  Their intended
meaning is unclear.

### The Alternate Character Set

For use in chtypes and with related functions. These are a portable way
to represent graphics characters on different terminals.

VT100-compatible symbols -- box characters:

   Name         | Description
   :------------|:-------------------------------------
   ACS_ULCORNER | upper left box corner
   ACS_LLCORNER | lower left box corner
   ACS_URCORNER | upper right box corner
   ACS_LRCORNER | lower right box corner
   ACS_RTEE     | right "T"
   ACS_LTEE     | left "T"
   ACS_BTEE     | bottom "T"
   ACS_TTEE     | top "T"
   ACS_HLINE    | horizontal line
   ACS_VLINE    | vertical line
   ACS_PLUS     | plus sign, cross, or four-corner piece

VT100-compatible symbols -- other:

   Name         | Description
   :------------|:-------------------------------------
   ACS_S1       | scan line 1
   ACS_S9       | scan line 9
   ACS_DIAMOND  | diamond
   ACS_CKBOARD  | checkerboard -- 50% grey
   ACS_DEGREE   | degree symbol
   ACS_PLMINUS  | plus/minus sign
   ACS_BULLET   | bullet

Teletype 5410v1 symbols -- these are defined in SysV curses, but are not
well-supported by most terminals. Stick to VT100 characters for optimum
portability:

   Name         | Description
   :------------|:-------------------------------------
   ACS_LARROW   | left arrow
   ACS_RARROW   | right arrow
   ACS_DARROW   | down arrow
   ACS_UARROW   | up arrow
   ACS_BOARD    | checkerboard -- less dense than ACS_CKBOARD
   ACS_LANTERN  | lantern symbol
   ACS_BLOCK    | solid block

That goes double for these -- undocumented SysV symbols. Don't use them:

   Name         | Description
   :------------|:-------------------------------------
   ACS_S3       | scan line 3
   ACS_S7       | scan line 7
   ACS_LEQUAL   | less than or equal
   ACS_GEQUAL   | greater than or equal
   ACS_PI       | pi
   ACS_NEQUAL   | not equal
   ACS_STERLING | pounds sterling symbol

Box character aliases:

   Name         | Description
   :------------|:-------------------------------------
   ACS_BSSB     | same as ACS_ULCORNER
   ACS_SSBB     | same as ACS_LLCORNER
   ACS_BBSS     | same as ACS_URCORNER
   ACS_SBBS     | same as ACS_LRCORNER
   ACS_SBSS     | same as ACS_RTEE
   ACS_SSSB     | same as ACS_LTEE
   ACS_SSBS     | same as ACS_BTEE
   ACS_BSSS     | same as ACS_TTEE
   ACS_BSBS     | same as ACS_HLINE
   ACS_SBSB     | same as ACS_VLINE
   ACS_SSSS     | same as ACS_PLUS

PDCursesMod-only ACS chars.  Don't use if compatibility with any other
curses implementation matters.  Some won't work in non-wide X11 builds
(see 'common/acs_defs.h' for details).  Best avoided.

   Name              | Description
   :-----------------|:-------------------------------
   ACS_CENT          | Cent symbol
   ACS_YEN           | Yen symbol
   ACS_PESETA        | Peseta symbol
   ACS_HALF          | 1/2 vulgar fraction
   ACS_QUARTER       | 1/4 vulgar fraction
   ACS_LEFT_ANG_QU   | Left-angle quote
   ACS_RIGHT_ANG_QU  | Right-angle quote
   ACS_CLUB          | Club (card suit)
   ACS_HEART         | Heart (card suit)
   ACS_SPADE         | Spade (card suit)
   ACS_SMILE         | Smile
   ACS_REV_SMILE     | Inverse-video smile
   ACS_MED_BULLET    | Medium bullet
   ACS_WHITE_BULLET  | Large bullet
   ACS_PILCROW       | Pilcrow (paragraph mark)
   ACS_SECTION       | Section mark
                     |
   ACS_SUP2          | Superscript 2
   ACS_ALPHA         | Lowercase alpha
   ACS_BETA          | Lowercase beta
   ACS_GAMMA         | Uppercase gamma
   ACS_UP_SIGMA      | Uppeccase sigma
   ACS_LO_SIGMA      | Lowercase sigma
   ACS_MU            | Lowercase mu
   ACS_TAU           | Lowercase tau
   ACS_UP_PHI        | Uppercase phi
   ACS_THETA         | Lowercase theta
   ACS_OMEGA         | Uppercase omega
   ACS_DELTA         | Lowercase delta
   ACS_INFINITY      | Infinity sign
   ACS_LO_PHI        | Lowercase phi
   ACS_EPSILON       | Lowercase epsilon
   ACS_INTERSECT     | Intersection symbol
   ACS_TRIPLE_BAR    | Triple bar
   ACS_DIVISION      | Division sign
   ACS_APPROX_EQ     | Approximate-equality sign
   ACS_SM_BULLET     | Small bullet
   ACS_SQUARE_ROOT   | Square root symbol
   ACS_UBLOCK        | Upper-half block
   ACS_BBLOCK        | Lower-half block
   ACS_LBLOCK        | Left-half block
   ACS_RBLOCK        | Right-half block
                     |
   ACS_A_ORDINAL     | a-ordinal sign
   ACS_O_ORDINAL     | o-ordinal sign
   ACS_INV_QUERY     | Inverted question mark
   ACS_REV_NOT       | Logical not sign,  mirrored
   ACS_NOT           | Logical not sign
   ACS_INV_BANG      | Inverted question mark
   ACS_UP_INTEGRAL   | Upper half of an integral sign
   ACS_LO_INTEGRAL   | Lower half of an integral sign
   ACS_SUP_N         | Superscript n
   ACS_CENTER_SQU    | Center square
   ACS_F_WITH_HOOK   | Hooked f


For cchar_t and wide-character functions, WACS_ equivalents are also
defined.

### Colors

For use with init_pair(), color_set(), etc.:

   Name         | BGR value | RGB value
   :------------|:---------:|:---------:
   COLOR_BLACK  |     0     |     0
   COLOR_BLUE   |     1     |     4
   COLOR_GREEN  |     2     |     2
   COLOR_CYAN   |     3     |     6
   COLOR_RED    |     4     |     1
   COLOR_MAGENTA|     5     |     5
   COLOR_YELLOW |     6     |     3
   COLOR_WHITE  |     7     |     7

Use the symbolic names instead of numeric values. The definition of the
colors depends on the implementation of curses.


### Input Values

The following constants might be returned by getch() if keypad() has
been enabled.  Note that not all of these may be supported on a
particular terminal:

   Name         | Description
   :------------|:-------------------------------------
   KEY_BREAK    | break key
   KEY_DOWN     | the four arrow keys
   KEY_UP       |
   KEY_LEFT     |
   KEY_RIGHT    |
   KEY_HOME     | home key (upward+left arrow)
   KEY_BACKSPACE| backspace
   KEY_F0       | function keys; space for 64 keys is reserved
   KEY_F(n)     | (KEY_F0+(n))
   KEY_DL       | delete line
   KEY_IL       | insert line
   KEY_DC       | delete character
   KEY_IC       | insert character
   KEY_EIC      | exit insert character mode
   KEY_CLEAR    | clear screen
   KEY_EOS      | clear to end of screen
   KEY_EOL      | clear to end of line
   KEY_SF       | scroll 1 line forwards
   KEY_SR       | scroll 1 line backwards (reverse)
   KEY_NPAGE    | next page
   KEY_PPAGE    | previous page
   KEY_STAB     | set tab
   KEY_CTAB     | clear tab
   KEY_CATAB    | clear all tabs
   KEY_ENTER    | enter or send
   KEY_SRESET   | soft (partial) reset
   KEY_RESET    | reset or hard reset
   KEY_PRINT    | print or copy
   KEY_LL       | home down or bottom (lower left)
   KEY_A1       | upper left of virtual keypad
   KEY_A3       | upper right of virtual keypad
   KEY_B2       | center of virtual keypad
   KEY_C1       | lower left of virtual keypad
   KEY_C3       | lower right of virtual keypad
                |
   KEY_BTAB     | Back tab key
   KEY_BEG      | Beginning key
   KEY_CANCEL   | Cancel key
   KEY_CLOSE    | Close key
   KEY_COMMAND  | Cmd (command) key
   KEY_COPY     | Copy key
   KEY_CREATE   | Create key
   KEY_END      | End key
   KEY_EXIT     | Exit key
   KEY_FIND     | Find key
   KEY_HELP     | Help key
   KEY_MARK     | Mark key
   KEY_MESSAGE  | Message key
   KEY_MOVE     | Move key
   KEY_NEXT     | Next object key
   KEY_OPEN     | Open key
   KEY_OPTIONS  | Options key
   KEY_PREVIOUS | Previous object key
   KEY_REDO     | Redo key
   KEY_REFERENCE| Reference key
   KEY_REFRESH  | Refresh key
   KEY_REPLACE  | Replace key
   KEY_RESTART  | Restart key
   KEY_RESUME   | Resume key
   KEY_SAVE     | Save key
   KEY_SBEG     | Shifted beginning key
   KEY_SCANCEL  | Shifted cancel key
   KEY_SCOMMAND | Shifted command key
   KEY_SCOPY    | Shifted copy key
   KEY_SCREATE  | Shifted create key
   KEY_SDC      | Shifted delete char key
   KEY_SDL      | Shifted delete line key
   KEY_SELECT   | Select key
   KEY_SEND     | Shifted end key
   KEY_SEOL     | Shifted clear line key
   KEY_SEXIT    | Shifted exit key
   KEY_SFIND    | Shifted find key
   KEY_SHELP    | Shifted help key
   KEY_SHOME    | Shifted home key
   KEY_SIC      | Shifted input key
   KEY_SLEFT    | Shifted left arrow key
   KEY_SMESSAGE | Shifted message key
   KEY_SMOVE    | Shifted move key
   KEY_SNEXT    | Shifted next key
   KEY_SOPTIONS | Shifted options key
   KEY_SPREVIOUS| Shifted prev key
   KEY_SPRINT   | Shifted print key
   KEY_SREDO    | Shifted redo key
   KEY_SREPLACE | Shifted replace key
   KEY_SRIGHT   | Shifted right arrow
   KEY_SRSUME   | Shifted resume key
   KEY_SSAVE    | Shifted save key
   KEY_SSUSPEND | Shifted suspend key
   KEY_SUNDO    | Shifted undo key
   KEY_SUSPEND  | Suspend key
   KEY_UNDO     | Undo key

The virtual keypad is arranged like this:

    A1     up     A3
    left   B2  right
    C1    down    C3

This list is incomplete -- see curses.h for the full list, and use the
testcurs demo to see what values are actually returned. The above are
just the keys required by X/Open. In particular, PDCurses defines many
CTL_ and ALT_ combinations; these are not portable.

[elsewhere]: MANUAL.md
