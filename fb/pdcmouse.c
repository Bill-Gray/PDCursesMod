#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <error.h>
#include <time.h>
#include <linux/input.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "pdcfb.h"

/* Code to use the Linux 'evdev' system to read the mouse (and,  eventually,
keyboard) in the console.  Built largely from information at

   https://thehackerdiary.wordpress.com/2017/04/21/exploring-devinput-1/
   https://stackoverflow.com/questions/26693280/linux-uinput-simple-example /
   https://cgit.freedesktop.org/evtest/tree/evtest.c

   Mouse and keyboard data can be found in one of the /dev/input/event# files.  The
only way I've found to determine which files to use is to look at all
of them sequentially until you find one that supports the BTN_LEFT (left
mouse button) event for the mouse,  or a KEY_A event for the keyboard.
(A one-button mouse returns a left click.)

   Also note that to get this access,  you must be a member of the 'input'
group (or run with root privileges,  but that'd be overdoing things.)

   Compile with gcc -Wall -Wextra -pedantic -o mouse3 mouse3.c -levdev
*/

#define bits_per_long (sizeof( long) << 3)
#define test_bit(bit, array)   ((array[bit / bits_per_long] >> (bit % bits_per_long)) & 1)

static int _get_mouse_fds( int *return_fds)
{
   int fd, n_found = 0;
   const char *directory = "/dev/input";
   DIR *dir_ptr = opendir( directory);
   struct dirent *direntp;

   if( !dir_ptr)
      return( -1);
   while( NULL != (direntp = readdir( dir_ptr)))
      if( !memcmp( direntp->d_name, "event", 5))
         {
         unsigned long bits[KEY_MAX / sizeof( long) + 1];
         char full_name[PATH_MAX];

         strcpy( full_name, directory);
         strcat( full_name, "/");
         strcat( full_name, direntp->d_name);
         fd = open( full_name, O_RDONLY | O_NONBLOCK);
         assert( fd > 0);
         if( fd > 0)
            {
            memset( bits, 0, sizeof( bits));
            ioctl(fd, EVIOCGBIT( 0, EV_MAX), bits);
            if( test_bit( EV_KEY, bits))
               {
               ioctl( fd, EVIOCGBIT( EV_KEY, KEY_MAX), bits);

               if( test_bit( BTN_LEFT, bits))
                  return_fds[n_found++] = fd;
               }
            else
               close( fd);
            }
         }
   closedir( dir_ptr);
   return n_found;
}

int PDC_update_mouse( void)
{
   static int n_fds = -1, fds[20];
   int i, rval = 0;

   if( n_fds == -1)
      n_fds = _get_mouse_fds( fds);
   for( i = 0; !rval && i < n_fds; i++)
      {
      struct input_event ev = {0};

      while( read(fds[i], &ev, sizeof(ev)) && EV_SYN != ev.type)
         if( EV_REL == ev.type &&
                        (ev.code == REL_X || ev.code == REL_Y))
            {
            if( ev.code == REL_X)
               PDC_mouse_x += ev.value;
            else
               PDC_mouse_y += ev.value;
            rval = 1;
            }
      }
   return( rval);
}
