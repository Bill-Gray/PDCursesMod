#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "curses.h"
#include "../common/xlates.h"

/* (Very) primitive terminal emulator using PDCursesMod.  Could use
PDCurses instead.  Or probably other implementations of curses.

   We set up a pseudterminal (pty).  Much of this comes from

https://learning.oreilly.com/library/view/the-linux-programming/9781593272203/xhtml/ch64.xhtml

   In this program,  we start a child process (by default,  /bin/bash -i).
The parent process connects to its stdin and stdout,  initializes curses,
and enters a loop.  If a character is received via getch( ),  it's
written to the child's stdin.  If characters are read from the child's
stdout,  we add them to the screen via addch( ).

   Note,  too,  that this is _not_ a full-fledged terminal emulator
(though I think it could be turned into one).  It handles the escape
sequence for setting the window title and a couple of other escape
sequences to delete characters;  that was just enough to avoid having
a window full of junk. You could call this a VT0 emulator... hence the
file name.

   It works decently on SDLn,  x11new,  and Linux framebuffer (DRM).

   I could imagine something similar allowing us to build a Windows console.
*/

int     PDC_mbtowc(wchar_t *, const char *, size_t);

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
      struct winsize ws;

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

      ws.ws_row = 25;
      ws.ws_col = 80;
      ioctl( term_fd, TIOCSWINSZ, &ws);

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

      /* close unused file descriptors, these are for child only : */
      fcntl( master_fd, F_SETFL, O_NONBLOCK);

      nocbreak( );
      noecho( );
      clear( );
      keypad(stdscr, TRUE);
      start_color( );
      addstr( "Welcome to PDCursesModTerm!\n");
      setbuf( stdout, NULL);
      raw( );
      scrollok( stdscr, TRUE);
      refresh( );
      timeout( 20);

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
                           buff[i] = '\0';
                           switch( buff[i - 1])
                              {
                              case 'K':
                                 delch( );
                                 break;
                              case 'P':
                                 delch( );
                                 break;
                              default:
                                 break;
                              }
                              break;
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
