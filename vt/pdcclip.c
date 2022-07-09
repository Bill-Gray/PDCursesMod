#ifdef _WIN32
   #include <windows.h>
   #undef MOUSE_MOVED
   #include <curspriv.h>
   #include "../common/winclip.c"
#else
   #include "../common/pdcclip.c"
#endif
