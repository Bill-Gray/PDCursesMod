#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <assert.h>
#include <stdbool.h>

/* Separable PDC clipboard functions for X11.  These can be used
without bringing in the rest of PDCurses.  They can also be used
on the VT platform (if you've got X11 going) and in wmcbrine's
'x11new' platform.  In both cases,  you have to add -pthreads to
the Makefile.

   Programs using PDC_setclipboard should link with -lX11 -lpthread.
Programs only using PDC_getclipboard will only need -lX11 (no threading).

   Much of this was copied from :

https://stackoverflow.com/questions/27378318/c-get-string-from-clipboard-on-linux
https://github.com/exebook/x11clipboard

   The X11 clipboard is display-specific,  so our first task is to open
the root display.  The clipboard is "owned" by a window,  so we make a
dummy window,  and try to set it as the selection owner.  That may
conceivably fail,  so we check to make sure we really got it.

   Then,  if copying,  we go into a loop waiting for somebody to request
the selection. If they do,  and they're looking for UTF8 or ASCII text,
we give them what they're looking for.  If we're notified that the
selection has been cleared (somebody else wants to own it),  we're done
and return 0.

   Ideally,  this whole looping-waiting-for-requests should take place in
a separate thread so that the rest of our program can go on with its
work,  and the following 'XCopy_threaded' does just that.  The loop
exits when we get a 'selection cleared' message.  Close down your program,
and the thread shuts down and the selection is lost... but that's par for
the course in X;  to evade it,  use a clipboard manager.  (Which will
notice right away that you own the selection;   it will then request the
selection,  make a copy,  and claim ownership itself.  So our thread will
have a very short life.)

   'cliptype' should be either PRIMARY or CLIPBOARD.   */

static int XCopy( char *text, const char *cliptype, long length);
int XCopy_threaded( const char *text, const char *cliptype);

static int XCopy( char *text, const char *cliptype, long length)
{
   Display *display = XOpenDisplay( 0);
   int N = DefaultScreen( display);
   unsigned long color = BlackPixel( display, N);
   Window window = XCreateSimpleWindow( display, RootWindow( display, N),
              0, 0, 1, 1, 0, color, color);
   Atom UTF8 = XInternAtom( display, "UTF8_STRING", 1);
   const Atom XA_ATOM = 4, XA_STRING = 31;
   const Atom targets_atom = XInternAtom( display, "TARGETS", 0);
   const Atom text_atom = XInternAtom(display, "TEXT", 0);
   const Atom selection = XInternAtom(display, cliptype, 0);
   XEvent event;
   int rval = 0;

   if (UTF8 == None)
       UTF8 = XA_STRING;
   XSetSelectionOwner( display, selection, window, 0);
   if( XGetSelectionOwner( display, selection) != window)
      rval = -1;
   else if( text) do
   {
      XNextEvent( display, &event);
      if( event.type == SelectionRequest &&
               event.xselectionrequest.selection == selection)
      {
         XSelectionRequestEvent * xsr = &event.xselectionrequest;
         XSelectionEvent ev = {0};
         int R = 0;

         ev.type = SelectionNotify, ev.display = xsr->display,
         ev.requestor = xsr->requestor, ev.selection = xsr->selection,
         ev.time = xsr->time, ev.target = xsr->target, ev.property = xsr->property;
         if (ev.target == targets_atom)
            R = XChangeProperty (ev.display, ev.requestor, ev.property, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&UTF8, 1);
         else if (ev.target == XA_STRING || ev.target == text_atom)
            R = XChangeProperty(ev.display, ev.requestor, ev.property, XA_STRING, 8,
                        PropModeReplace, (const unsigned char *)text, length);
         else if (ev.target == UTF8)
            R = XChangeProperty(ev.display, ev.requestor, ev.property, UTF8, 8,
                        PropModeReplace, (const unsigned char *)text, length);
         else
            ev.property = None;
         if ((R & 2) == 0)
            XSendEvent (display, ev.requestor, 0, 0, (XEvent *)&ev);
      }
   } while( event.type != SelectionClear);
   XDestroyWindow( display, window);
   XCloseDisplay( display);
   free( text);
   return( rval);
}

#include <pthread.h>

static void *XCopy_thread_func( void *clip_request)
{
   char **args = (char **)clip_request;
   const long local_length = (const long)args[2];

   assert( args);
   assert( args[0]);
   assert( args[1]);
   XCopy( args[0], args[1], local_length);
   return( NULL);
}

static int setclipboard( const char *contents, const long length,
                                                const bool is_select)
{
   int rval = 0;
   static pthread_t thread_id;
   static char *args[4];

   if( contents)
      {
      args[0] = (char *)malloc( length + 1);
      args[0][length] = '\0';
      memcpy( args[0], contents, length);
      }
   else
      args[0] = NULL;
   args[1] = (is_select ? "PRIMARY" : "CLIPBOARD");
   args[2] = (char *)length;
   args[3] = NULL;
   if( thread_id)
      {
      XCopy( NULL, args[1], 0);
      pthread_join( thread_id, NULL);
      thread_id = (pthread_t)0;
      }
   if( contents)
      rval = pthread_create( &thread_id, NULL, XCopy_thread_func, args);
   return( rval);
}

int PDC_setclipboard( const char *contents, long length)
{
   return( setclipboard( contents, length, false));
}

int PDC_setselection( const char *contents, long length)
{
   return( setclipboard( contents, length, true));
}

static int getclipboard( char **contents, long *length, const bool is_select)
{
   Display *display = XOpenDisplay( 0);
   int N = DefaultScreen( display);
   unsigned long color = BlackPixel( display, N);
   Window window = XCreateSimpleWindow( display, RootWindow( display, N),
              0, 0, 1, 1, 0, color, color);
   Atom UTF8 = XInternAtom( display, "UTF8_STRING", 1);
   const Atom XA_STRING = 31;
   char *result, *rval = NULL;
   unsigned long ressize = 0, restail;
   int resbits;
   const Atom bufid = XInternAtom(display,
                          (is_select ? "PRIMARY" : "CLIPBOARD"), False),
        propid = XInternAtom(display, "XSEL_DATA", False),
        incrid = XInternAtom(display, "INCR", False);
   XEvent event;

   if (UTF8 == None)
       UTF8 = XA_STRING;
   XConvertSelection(display, bufid, UTF8, propid, window, CurrentTime);
   do {
      XNextEvent(display, &event);
   } while (event.type != SelectionNotify || event.xselection.selection != bufid);

   if (event.xselection.property)
      {
      XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, False, AnyPropertyType,
             &UTF8, &resbits, &ressize, &restail, (unsigned char**)&result);

      if( UTF8 != incrid)
         {
         rval = (char *)malloc( ressize + 1);
         memcpy( rval, result, ressize);
         rval[ressize] = '\0';
         }
      XFree(result);
      XDeleteProperty( display, window, propid);
      }
   XDestroyWindow(display, window);
   XCloseDisplay(display);
   *contents = rval;
   *length = (long)ressize;
   return( rval ? 0 : -1);
}

int PDC_getclipboard( char **contents, long *length)
{
   return( getclipboard( contents, length, false));
}

int PDC_getselection( char **contents, long *length)
{
   return( getclipboard( contents, length, true));
}

int PDC_freeclipboard( char *contents)
{
   free( contents);
   return( 0);
}

int PDC_clearclipboard( void)
{
   return( 0);
}
