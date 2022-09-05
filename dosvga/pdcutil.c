/* PDCurses */

#include "pdcdos.h"
#include "../common/dosutil.c"

const char *PDC_sysname(void)
{
    return "DOSVGA";
}

enum PDC_port PDC_port_val = PDC_PORT_DOSVGA;
