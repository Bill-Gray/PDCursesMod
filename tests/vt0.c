#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "curses.h"
#include "../common/xlates.h"

/* (Very) primitive *nix terminal emulator using PDCursesMod.  Could
use PDCurses instead,  with minor changes required for color handling.
Other implementations of curses would probably work as well.

   We set up a pseudoterminal (pty).  Much of this comes from

https://learning.oreilly.com/library/view/the-linux-programming/9781593272203/xhtml/ch64.xhtml

   In this program,  we start a child process (by default,  /bin/bash -i).
The parent process connects to its stdin and stdout,  initializes curses,
and enters a loop.  If a character is received via getch( ),  it's
written to the child's stdin.  If characters are read from the child's
stdout,  we add them to the screen via addch( ).

   Note,  too,  that this is _not_ a full-fledged terminal emulator
(though I think it could be turned into one).  It handles the escape
sequence for setting the window title and a few escape sequences to
delete characters, set colors,  and so on.  Screen resizing is not well
handled.   You could call this a VT0 emulator... hence the file name.

   It works decently on SDLn,  x11new,  and Linux framebuffer/DRM.

   I could imagine something similar allowing us to build a Windows console.
*/

int     PDC_mbtowc(wchar_t *, const char *, size_t);

/* Many VTn control sequences have a series of integer parameters,  separated
by colons or semicolons.  get_param( ) evaluates and returns one such parameter,
advancing the pointer to the next parameter (if any),  and returning -1 if
we've run out of parameters (or hit an error).        */

static int get_param( const char **iptr)
{
   const char *tptr = *iptr;
   int rval;

   if( !isdigit( *tptr))
      rval = -1;
   else
      {
      rval = *tptr++ - '0';
      while( isdigit( *tptr))
         rval = rval * 10 + *tptr++ - '0';
      if( *tptr == ':' || *tptr == ';')
         tptr++;
      }
   *iptr = tptr;
   return( rval);
}

int create_term( const char* szCommand, const char **args,
                 const char **environ)
{
   const int master_fd = posix_openpt( O_RDWR | O_NOCTTY);
   int pid_child;

   if( master_fd < 0)
      {
      perror( "couldn't open pseudoterminal");
      return -1;
      }

   if( -1 == grantpt( master_fd) || -1 == unlockpt( master_fd))
      {
      close( master_fd);
      return -1;
      }

   pid_child = fork();
   if( !pid_child)    /* child continues here */
      {
      char *name = ptsname( master_fd);
      int term_fd;
      struct termios term;

      if (setsid() == -1)
         return( -1);
      term_fd = open( name, O_RDWR);
      if( -1 == term_fd)
         return( -2);
#ifdef TIOCSCTTY                 /* acquire controlling tty on BSD */
      if (ioctl( term_fd, TIOCSCTTY, 0) == -1)
         return( -3);
#endif

      tcgetattr( term_fd, &term);
      term.c_oflag &= ~OPOST;
      term.c_iflag &= ~ICRNL;
      tcsetattr( term_fd, TCSANOW, &term);

      /* redirect stdin,  stdout,  stderr  */
      if( -1 == dup2( term_fd, STDIN_FILENO)
           || -1 == dup2( term_fd, STDOUT_FILENO)
           || -1 == dup2( term_fd, STDERR_FILENO))
         exit(errno);

      execve(szCommand, (char * const *) args, (char * const *)environ);

      /* if we get here at all, an error occurred, but we are */
      /* in the child process, so just exit                   */
      perror( "Child process failed");
      exit( -1);
      }
  else if( pid_child > 0)        /* parent continues here */
      {
      int wait_status;
      SCREEN *screen_pointer = newterm(NULL, stdout, stdin);
      struct winsize ws;

      /* close unused file descriptors, these are for child only : */
      fcntl( master_fd, F_SETFL, O_NONBLOCK);

      nocbreak( );
      noecho( );
      clear( );
      keypad(stdscr, TRUE);
      start_color( );
      use_default_colors( );
      addstr( "Welcome to PDCursesModTerm!\n");
      setbuf( stdout, NULL);
      raw( );
      scrollok( stdscr, TRUE);
      refresh( );
      timeout( 20);

      ws.ws_row = LINES;
      ws.ws_col = COLS;
      ioctl( master_fd, TIOCSWINSZ, &ws);

      while( waitpid( pid_child, &wait_status, WNOHANG) >= 0)
         {
         char nChar;
         int ch;

         while( 1 == read( master_fd, &nChar, 1))
            {
            if( nChar != 27)
               {
               static int buffsize = 0;

               if( !(nChar & 0x80))
                  {
                  if( nChar != 7)      /* Ctrl-G = 'bell' */
                     addch( nChar);
                  buffsize = 0;
                  }
               else
                  {
                  static char utf8_bytes[5];
                  wchar_t result;

                  assert( buffsize < 5);
                  utf8_bytes[buffsize++] = nChar;
                  if( PDC_mbtowc( &result, utf8_bytes, buffsize) > 0)
                     {
                     addch( (chtype)result);
                     buffsize = 0;
                     }
                  }
               }
            else if( 1 == read( master_fd, &nChar, 1))
               {
               char buff[200];
               size_t i = 0;

               switch( nChar)
                  {
                  case '[' :            /* Control Sequence Introducer (CSI) */
                     {
                     while( i < sizeof( buff) - 1
                            && 1 == read( master_fd, buff + i, 1))
                        {
                        i++;
                        if( buff[i - 1] >= '@')    /* valid CSI sequence */
                           {
                           int param = atoi( buff + ((*buff == '?') ? 1 : 0));

                           buff[i] = '\0';
                           switch( buff[i - 1])
                              {
                              case 'H':
                                 {
                                 int row, col;

                                 if( sscanf( buff, "%d;%d", &row, &col) == 2)
                                    move( row - 1, col - 1);
                                 }
                                 break;
                              case 'G':
                                 move( getcury( stdscr), (param > 1 ? param - 1 : 0));
                                 break;
                              case 'K':
                                 clrtoeol( );
                                 break;
                              case 'J':
                                 switch( buff[i - 2])
                                    {
                                    case '1':
                                       /* erase above... TBD */
                                       break;
                                    case '2':
                                       clear( );
                                       break;
                                    default:
                                       clrtobot( );
                                       break;
                                    }
                                 break;
                              case 'P':
                              case 'X':
                                 {
                                 if( param < 1)
                                    param = 1;
                                 while( param--)
                                    delch( );
                                 }
                                 break;
                              case 'm':       /* color(s)/attribute(s) */
                                 {
                                 int ival;
                                 const char *tptr = buff;
                                 static int fg = -1, bg = -1;
                                 const int old_bg = bg, old_fg = fg;

                                 while( (ival = get_param( &tptr)) >= 0)
                                    {
                                    const attr_t att[9] = { A_BOLD, A_DIM, A_ITALIC,
                                                A_UNDERLINE, A_BLINK, 0, A_REVERSE,
                                                A_INVIS, A_STRIKEOUT };

                                    if( !ival)
                                       {
                                       attrset( 0);
                                       fg = bg = -1;
                                       }
                                    else if( ival < 10)
                                       attron( att[ival - 1]);
                                    else if( ival < 22)
                                       ;            /* shouldn't happen */
                                    else if( ival < 30)
                                       attroff( att[ival - 21]);
                                    else if( ival < 38)
                                       fg = ival - 30;
                                    else if( ival == 38 || ival == 48)   /* true-color handling */
                                       {
                                       const int val2 = get_param( &tptr);
                                       const int val3 = get_param( &tptr);
                                       int new_idx = -1;

                                       if( val2 == 5 && val3 >= 0)
                                          new_idx = val3;   /* color selected by index */
                                       else if( val2 == 2 && val3 >= 0)
                                          {
                                          const int val4 = get_param( &tptr);
                                          const int val5 = get_param( &tptr);

                                          new_idx = val3  | (val4 << 8) | (val5 << 16);
                                          new_idx += 256;
                                          }
                                       if( new_idx >= 0)
                                          {
                                          if( ival == 38)
                                             fg = new_idx;
                                          else
                                             bg = new_idx;
                                          }
                                       }
                                    else if( ival == 39)
                                       fg = -1;            /* foreground to default */
                                    else if( ival < 48)
                                       bg = ival - 40;
                                    else if( ival == 49)
                                       bg = -1;            /* foreground to default */
                                    }
                                 if( fg != old_fg || bg != old_bg)
                                    {
                                    const int pair_idx = (fg == -1 && bg == -1) ? 0 :
                                                 alloc_pair( fg, bg);

                                    color_set( pair_idx, NULL);
                                    }
                                 }
                                 break;
                              default:
                                 break;
                              }
                              break;
                           i = 0;      /* reset for another possible sequence */
                           }
                        }
                     }
                     break;
                  case ']' :            /* Operating System Command (OSC) */
                     if( 1 == read( master_fd, &nChar, 1))
                        {
                        switch( nChar)
                           {
                           case '0':      /* set window/icon title */
                           case '1':      /* set icon label */
                           case '2':      /* set window title */
                              if( 1 == read( master_fd, &nChar, 1) && nChar == ';')
                                 {
                                 char title[255];

                                 while( i < sizeof( title) &&
                                           1 == read( master_fd, title + i, 1))
                                    if( '\a' == title[i])
                                       {
                                       title[i] = '\0';
                                       PDC_set_title( title);
                                       break;
                                       }
                                    else
                                       i++;
                                 }
                               break;
                           default:
                               while( 1 == read( master_fd, &nChar, 1))
                                 ;
                               break;
                           }
                        }
                     break;
                  case '(':         /* designate character set : not used yet */
                     if( read( master_fd, &nChar, 1) == 1)
                        if( nChar == '%' || nChar == '"' || nChar == '&')
                           {
                           int n_read = read( master_fd, &nChar, 1);

                           assert( 1 == n_read);
                           }
                     break;
                  default:
                     break;
                  }
               }
            }
         refresh( );
         while( ERR != (ch = getch( )))
            {
            if( ch > 0 && ch < 127)
               {
               nChar = (char)ch;
               if( !write( master_fd, &nChar, 1))
                  exit( -1);
               }
            else if( ch == KEY_RESIZE)
               {
               ws.ws_row = LINES;
               ws.ws_col = COLS;
               if( ioctl( master_fd, TIOCSWINSZ, &ws) == -1)
                  printf( "Failed to set\n");
               }
            else for( size_t i = 0; i < sizeof( xlates) / sizeof( xlates[0]); i++)
               if( xlates[i].key_code == ch)
                  {
                  nChar = 27;
                  if( !write( master_fd, &nChar, 1))
                     exit( -1);
                  if( !write( master_fd, xlates[i].xlation, strlen( xlates[i].xlation)))
                     exit( -1);
                  break;
                  }
            }
         }

      endwin();
                             /* Not really needed,  but ensures Valgrind  */
      delscreen( screen_pointer);          /* says all memory was freed */
      }
   else                   /* pid_child < 0;  the fork failed */
      {
      }
  return pid_child;
}


int main( const int argc, const char **argv)
{
   extern const char **environ;

   if( argc < 2)
      {
      static const char *arguments[4] = { NULL, "/bin/bash", "-i", NULL };

      argv = arguments;
      }
   create_term( argv[1], &argv[1], environ);

   return( 0);
}
