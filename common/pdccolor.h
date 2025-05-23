#ifndef PDCCOLOR_H

#define PDCCOLOR_H

typedef uint32_t PACKED_RGB;

#define Get_BValue( rgb) ((int)( (rgb) >> 16))
#define Get_GValue( rgb) ((int)( (rgb) >> 8) & 0xff)
#define Get_RValue( rgb) ((int)((rgb) & 0xff))

int PDC_init_palette( void);
void PDC_get_rgb_values( const chtype srcp,
            PACKED_RGB *foreground_rgb, PACKED_RGB *background_rgb);
int PDC_set_palette_entry( const int idx, const PACKED_RGB rgb);
PACKED_RGB PDC_get_palette_entry( const int idx);
void PDC_free_palette( void);
#endif
