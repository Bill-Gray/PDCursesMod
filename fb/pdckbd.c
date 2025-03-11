#define LINUX_FRAMEBUFFER_PORT

void PDC_check_for_blinking( void);
int PDC_cycle_font( void);
void PDC_rotate_font( void);

#ifdef USE_DRM
int PDC_cycle_display( void);
#endif

#include "../vt/pdckbd.c"
