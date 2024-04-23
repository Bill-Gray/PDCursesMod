#define LINUX_FRAMEBUFFER_PORT

void PDC_check_for_blinking( void);
int PDC_cycle_font( void);
void PDC_rotate_font( void);

#include "../vt/pdckbd.c"
