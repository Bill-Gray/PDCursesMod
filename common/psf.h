struct font_info {
        uint32_t font_type;
        uint32_t headersize;    /* offset of bitmaps in file */
        uint32_t n_glyphs;
        uint32_t charsize;      /* number of bytes for each character */
        uint32_t height, width; /* max dimensions of glyphs */
        uint32_t *unicode_info;
        uint32_t unicode_info_size;
        const uint8_t *glyphs;
};

int load_psf_or_vgafont( struct font_info *f, const uint8_t *buff, const long filelen);
int find_psf_or_vgafont_glyph( struct font_info *f, const uint32_t unicode_point);
