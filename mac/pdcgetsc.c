#include "pdcmac.h"

int PDC_get_cursor_mode(void)
{
    PDC_LOG(("PDC_get_cursor_mode() - called\n"));
    return 0;
}

int PDC_get_rows(void)
{
    PDC_LOG(("PDC_get_rows() - called\n"));
    return PDC_rows;
}

int PDC_get_columns(void)
{
    PDC_LOG(("PDC_get_columns() - called\n"));
    return PDC_cols;
}
