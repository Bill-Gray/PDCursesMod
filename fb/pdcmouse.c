#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <error.h>
#include <time.h>
#if defined( __has_include)
   #if __has_include(<linux/input.h>)
      #include <linux/input.h>
   #endif
#endif
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
group (or run with root privileges,  but that'd be overdoing things.)  */

#define bits_per_long (sizeof( long) << 3)
#define test_bit(bit, array)   ((array[bit / bits_per_long] >> (bit % bits_per_long)) & 1)

/* Unfortunately,  Curses has a _lot_ of overlapping #defines with the
Linux input system.  So we can't #include <curses.h> here.  Instead,  the
following #defines are cherry-picked.  If they're ever re-defined in Curses,
chaos will ensue.       */

#define BUTTON_RELEASED         0x0000
#define BUTTON_PRESSED          0x0001
#define BUTTON_MOVED            0x0005  /* PDCurses */
#define PDC_MOUSE_WHEEL_UP      0x0020
#define PDC_MOUSE_WHEEL_DOWN    0x0040
#define PDC_MOUSE_WHEEL_LEFT    0x0080
#define PDC_MOUSE_WHEEL_RIGHT   0x0100

#define PDC_KEY_MODIFIER_SHIFT   1
#define PDC_KEY_MODIFIER_CONTROL 2
#define PDC_KEY_MODIFIER_ALT     4
#define PDC_KEY_MODIFIER_NUMLOCK 8
#define PDC_KEY_MODIFIER_REPEAT  16


static int _get_mouse_or_keyboard_fds( int *return_fds, const int is_keyboard)
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
         char full_name[60];

         strcpy( full_name, directory);
         strcat( full_name, "/");
         assert( strlen( direntp->d_name) < 30);
         strcat( full_name, direntp->d_name);
         fd = open( full_name, O_RDONLY | O_NONBLOCK);
         assert( fd > 0);
         if( fd > 0)
            {
            memset( bits, 0, sizeof( bits));
            ioctl(fd, EVIOCGBIT( 0, EV_MAX), bits);
            if( test_bit( EV_KEY, bits))
               {
               const int bit_to_test = (is_keyboard ? KEY_A : BTN_LEFT);

               ioctl( fd, EVIOCGBIT( EV_KEY, KEY_MAX), bits);

               if( test_bit( bit_to_test, bits))
                  return_fds[n_found++] = fd;
               }
            else
               close( fd);
            }
         }
   closedir( dir_ptr);
   return n_found;
}

int PDC_get_modifiers( void)
{
   static int n_fds = -1, fds[20];
   int i;
   static int modifiers = 0;

   if( n_fds == -1)
      n_fds = _get_mouse_or_keyboard_fds( fds, 1);
   for( i = 0; i < n_fds; i++)
      {
      struct input_event ev = {0};

      while( read(fds[i], &ev, sizeof(ev)) == sizeof( ev))
         if( EV_KEY == ev.type && (ev.value == 0 || ev.value == 1))
            {
            int mask;

            switch( ev.code)
               {
               case KEY_LEFTSHIFT:
               case KEY_RIGHTSHIFT:
                  mask = PDC_KEY_MODIFIER_SHIFT;
                  break;
               case KEY_LEFTALT:
               case KEY_RIGHTALT:
                  mask = PDC_KEY_MODIFIER_ALT;
                  break;
               case KEY_LEFTCTRL:
               case KEY_RIGHTCTRL:
                  mask = PDC_KEY_MODIFIER_CONTROL;
                  break;
               default:
                  mask = 0;
               }
            if( mask)
               {
               if( ev.value)
                  modifiers |= mask;
               else
                  modifiers &= ~mask;
               }
            }
      }
   return( modifiers);
}

int PDC_update_mouse( int *button)
{
   static int n_fds = -1, fds[20];
   int i, rval = -1;

   if( n_fds == -1)
      n_fds = _get_mouse_or_keyboard_fds( fds, 0);
   for( i = 0; rval < 0 && i < n_fds; i++)
      {
      struct input_event ev = {0};

      while( rval < 0 && sizeof( ev) == read(fds[i], &ev, sizeof(ev)))
         if( EV_REL == ev.type &&
                        (ev.code == REL_X || ev.code == REL_Y))
            {
            if( ev.code == REL_X)
               PDC_mouse_x += ev.value;
            else if( ev.code == REL_Y)
               PDC_mouse_y += ev.value;
                        /* else it's a scroll-wheel event */
            rval = BUTTON_MOVED;
            }
         else if( EV_KEY == ev.type &&
                    ev.code >= BTN_LEFT && ev.code <= BTN_TASK)
            {
            *button = ev.code - BTN_LEFT + 1;
            if( *button == 2 || *button == 3)      /* Linux input system swaps */
               *button = 5 - *button;           /* these buttons relative to curses */
            rval = (ev.value ? BUTTON_PRESSED : BUTTON_RELEASED);
            }
         else if( EV_REL == ev.type && ev.code == REL_WHEEL_HI_RES)
            rval = (ev.value > 0 ? PDC_MOUSE_WHEEL_UP : PDC_MOUSE_WHEEL_DOWN);
         else if( EV_REL == ev.type && ev.code == REL_HWHEEL_HI_RES)
            rval = (ev.value > 0 ? PDC_MOUSE_WHEEL_RIGHT : PDC_MOUSE_WHEEL_LEFT);
      }
   return( rval);
}
