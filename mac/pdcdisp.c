#include "pdcmac.h"
#include <string.h>

#define USE_UNICODE_ACS_CHARS 1

#include <acs_defs.h>
#include <pdccolor.h>
#include <blink.c>

#ifndef PACK_RGB
#define PACK_RGB(red, green, blue) ((PACKED_RGB)(red) | ((PACKED_RGB)(green) << 8) | ((PACKED_RGB)(blue) << 16))
#endif

#define BOX_LIGHT 1
#define BOX_HEAVY 2
#define BOX_DOUBLE 3
#define BOX_ARMS(up, down, left, right) (((up) << 6) | ((down) << 4) | ((left) << 2) | (right))

static void packed_to_rgb(PACKED_RGB packed, RGBColor *color)
{
    unsigned int red;
    unsigned int green;
    unsigned int blue;

    red = Get_RValue(packed);
    green = Get_GValue(packed);
    blue = Get_BValue(packed);
    color->red = (unsigned short)((red << 8) | red);
    color->green = (unsigned short)((green << 8) | green);
    color->blue = (unsigned short)((blue << 8) | blue);
}

static void fill_rect(Rect *cell, const RGBColor *color)
{
    RGBForeColor(color);
    PaintRect(cell);
}

static int box_arms(unsigned long ch)
{
    static const struct
    {
        unsigned short code;
        unsigned char arms;
    } box_table[] =
    {
        { 0x2500, BOX_ARMS(0, 0, 1, 1) }, { 0x2501, BOX_ARMS(0, 0, 2, 2) },
        { 0x2502, BOX_ARMS(1, 1, 0, 0) }, { 0x2503, BOX_ARMS(2, 2, 0, 0) },
        { 0x250C, BOX_ARMS(0, 1, 0, 1) }, { 0x250F, BOX_ARMS(0, 2, 0, 2) },
        { 0x2510, BOX_ARMS(0, 1, 1, 0) }, { 0x2513, BOX_ARMS(0, 2, 2, 0) },
        { 0x2514, BOX_ARMS(1, 0, 0, 1) }, { 0x2517, BOX_ARMS(2, 0, 0, 2) },
        { 0x2518, BOX_ARMS(1, 0, 1, 0) }, { 0x251B, BOX_ARMS(2, 0, 2, 0) },
        { 0x251C, BOX_ARMS(1, 1, 0, 1) }, { 0x2523, BOX_ARMS(2, 2, 0, 2) },
        { 0x2524, BOX_ARMS(1, 1, 1, 0) }, { 0x252B, BOX_ARMS(2, 2, 2, 0) },
        { 0x252C, BOX_ARMS(0, 1, 1, 1) }, { 0x2533, BOX_ARMS(0, 2, 2, 2) },
        { 0x2534, BOX_ARMS(1, 0, 1, 1) }, { 0x253B, BOX_ARMS(2, 0, 2, 2) },
        { 0x253C, BOX_ARMS(1, 1, 1, 1) }, { 0x254B, BOX_ARMS(2, 2, 2, 2) },
        { 0x2550, BOX_ARMS(0, 0, 3, 3) }, { 0x2551, BOX_ARMS(3, 3, 0, 0) },
        { 0x2552, BOX_ARMS(0, 1, 0, 3) }, { 0x2553, BOX_ARMS(0, 3, 0, 1) },
        { 0x2554, BOX_ARMS(0, 3, 0, 3) }, { 0x2555, BOX_ARMS(0, 1, 3, 0) },
        { 0x2556, BOX_ARMS(0, 3, 1, 0) }, { 0x2557, BOX_ARMS(0, 3, 3, 0) },
        { 0x2558, BOX_ARMS(1, 0, 0, 3) }, { 0x2559, BOX_ARMS(3, 0, 0, 1) },
        { 0x255A, BOX_ARMS(3, 0, 0, 3) }, { 0x255B, BOX_ARMS(1, 0, 3, 0) },
        { 0x255C, BOX_ARMS(3, 0, 1, 0) }, { 0x255D, BOX_ARMS(3, 0, 3, 0) },
        { 0x255E, BOX_ARMS(1, 1, 0, 3) }, { 0x255F, BOX_ARMS(3, 3, 0, 1) },
        { 0x2560, BOX_ARMS(3, 3, 0, 3) }, { 0x2561, BOX_ARMS(1, 1, 3, 0) },
        { 0x2562, BOX_ARMS(3, 3, 1, 0) }, { 0x2563, BOX_ARMS(3, 3, 3, 0) },
        { 0x2564, BOX_ARMS(0, 1, 3, 3) }, { 0x2565, BOX_ARMS(0, 3, 1, 1) },
        { 0x2566, BOX_ARMS(0, 3, 3, 3) }, { 0x2567, BOX_ARMS(1, 0, 3, 3) },
        { 0x2568, BOX_ARMS(3, 0, 1, 1) }, { 0x2569, BOX_ARMS(3, 0, 3, 3) },
        { 0x256A, BOX_ARMS(1, 1, 3, 3) }, { 0x256B, BOX_ARMS(3, 3, 1, 1) },
        { 0x256C, BOX_ARMS(3, 3, 3, 3) }, { 0x256D, BOX_ARMS(0, 1, 0, 1) },
        { 0x256E, BOX_ARMS(0, 1, 1, 0) }, { 0x256F, BOX_ARMS(1, 0, 1, 0) },
        { 0x2570, BOX_ARMS(1, 0, 0, 1) }
    };
    int i;

    if (ch < 0x2500 || ch > 0x2570)
        return 0;
    for (i = 0; i < (int)(sizeof(box_table) / sizeof(box_table[0])); i++)
        if (box_table[i].code == ch)
            return box_table[i].arms;
    return 0;
}

static void box_line(int x1, int y1, int x2, int y2, int weight)
{
    if (weight == BOX_HEAVY)
        PenSize(2, 2);
    MoveTo(x1, y1);
    LineTo(x2, y2);
    PenSize(1, 1);
}

static int box_single_end(int weight_here, int weight_opposite, int both_across,
                          int center, int nearline, int farline, int across_double)
{
    if (!across_double)
        return center;
    if (both_across && !weight_opposite)
        return nearline;
    if (weight_here)
        return farline;
    return center;
}

static void draw_box(int arms, const Rect *cell)
{
    int up;
    int down;
    int left;
    int right;
    int midx;
    int midy;
    int top;
    int bottom;
    int leftx;
    int rightx;
    int vdouble;
    int hdouble;

    up = (arms >> 6) & 3;
    down = (arms >> 4) & 3;
    left = (arms >> 2) & 3;
    right = arms & 3;
    midx = (cell->left + cell->right) / 2;
    midy = (cell->top + cell->bottom) / 2;
    top = cell->top;
    bottom = cell->bottom - 1;
    leftx = cell->left;
    rightx = cell->right - 1;
    vdouble = up == BOX_DOUBLE || down == BOX_DOUBLE;
    hdouble = left == BOX_DOUBLE || right == BOX_DOUBLE;
    if ((vdouble || hdouble)
        && (!up || up == BOX_DOUBLE) && (!down || down == BOX_DOUBLE)
        && (!left || left == BOX_DOUBLE) && (!right || right == BOX_DOUBLE))
    {
        if (up)
        {
            box_line(midx - 1, top, midx - 1, midy - 1, BOX_LIGHT);
            box_line(midx + 1, top, midx + 1, midy - 1, BOX_LIGHT);
        }
        else
            box_line(midx - 1, midy - 1, midx + 1, midy - 1, BOX_LIGHT);
        if (down)
        {
            box_line(midx - 1, midy + 1, midx - 1, bottom, BOX_LIGHT);
            box_line(midx + 1, midy + 1, midx + 1, bottom, BOX_LIGHT);
        }
        else
            box_line(midx - 1, midy + 1, midx + 1, midy + 1, BOX_LIGHT);
        if (left)
        {
            box_line(leftx, midy - 1, midx - 1, midy - 1, BOX_LIGHT);
            box_line(leftx, midy + 1, midx - 1, midy + 1, BOX_LIGHT);
        }
        else
            box_line(midx - 1, midy - 1, midx - 1, midy + 1, BOX_LIGHT);
        if (right)
        {
            box_line(midx + 1, midy - 1, rightx, midy - 1, BOX_LIGHT);
            box_line(midx + 1, midy + 1, rightx, midy + 1, BOX_LIGHT);
        }
        else
            box_line(midx + 1, midy - 1, midx + 1, midy + 1, BOX_LIGHT);
        return;
    }
    if (up == BOX_DOUBLE || down == BOX_DOUBLE)
    {
        if (up)
        {
            box_line(midx - 1, top, midx - 1, midy, BOX_LIGHT);
            box_line(midx + 1, top, midx + 1, midy, BOX_LIGHT);
        }
        if (down)
        {
            box_line(midx - 1, midy, midx - 1, bottom, BOX_LIGHT);
            box_line(midx + 1, midy, midx + 1, bottom, BOX_LIGHT);
        }
    }
    else
    {
        if (up)
            box_line(midx, top, midx, box_single_end(up, down, left && right,
                     midy, midy - 1, midy + 1, hdouble), up);
        if (down)
            box_line(midx, box_single_end(down, up, left && right,
                     midy, midy + 1, midy - 1, hdouble), midx, bottom, down);
    }
    if (left == BOX_DOUBLE || right == BOX_DOUBLE)
    {
        if (left)
        {
            box_line(leftx, midy - 1, midx, midy - 1, BOX_LIGHT);
            box_line(leftx, midy + 1, midx, midy + 1, BOX_LIGHT);
        }
        if (right)
        {
            box_line(midx, midy - 1, rightx, midy - 1, BOX_LIGHT);
            box_line(midx, midy + 1, rightx, midy + 1, BOX_LIGHT);
        }
    }
    else
    {
        if (left)
            box_line(leftx, midy, box_single_end(left, right, up && down,
                     midx, midx - 1, midx + 1, vdouble), midy, left);
        if (right)
            box_line(box_single_end(right, left, up && down,
                     midx, midx + 1, midx - 1, vdouble), midy, rightx, midy, right);
    }
}

static void draw_small_pair(const Rect *cell, char first, char second)
{
    int size;

    size = PDC_font_size / 2 + 1;
    TextSize(size);
    MoveTo(cell->left, cell->top + size);
    DrawChar(first);
    if (second)
    {
        MoveTo(cell->right - CharWidth(second), cell->bottom - 1);
        DrawChar(second);
    }
    TextSize(PDC_font_size);
}

static void draw_face(const Rect *cell, int filled, const RGBColor *bg)
{
    Rect face;
    Rect eye;
    int diameter;
    int midx;
    int midy;

    diameter = cell->right - cell->left - 1;
    midx = (cell->left + cell->right) / 2;
    midy = (cell->top + cell->bottom) / 2;
    SetRect(&face, midx - diameter / 2, midy - diameter / 2,
            midx - diameter / 2 + diameter, midy - diameter / 2 + diameter);
    if (filled)
    {
        PaintOval(&face);
        RGBForeColor(bg);
    }
    else
        FrameOval(&face);
    SetRect(&eye, face.left + 2, face.top + 2, face.left + 3, face.top + 3);
    PaintRect(&eye);
    OffsetRect(&eye, diameter - 5, 0);
    PaintRect(&eye);
    MoveTo(face.left + 2, face.bottom - 3);
    LineTo(face.right - 3, face.bottom - 3);
}

static int draw_graphic(unsigned long ch, Rect *cell, const RGBColor *fg, const RGBColor *bg)
{
    int midx;
    int midy;
    int arms;
    Rect shape;

    midx = (cell->left + cell->right) / 2;
    midy = (cell->top + cell->bottom) / 2;
    RGBForeColor(fg);
    PenNormal();
    arms = box_arms(ch);
    if (arms)
    {
        draw_box(arms, cell);
        return 1;
    }
    shape = *cell;
    switch (ch)
    {
    case 0x2588:
        PaintRect(cell);
        return 1;
    case 0x2580:
        shape.bottom = midy;
        PaintRect(&shape);
        return 1;
    case 0x2584:
        shape.top = midy;
        PaintRect(&shape);
        return 1;
    case 0x258C:
        shape.right = midx;
        PaintRect(&shape);
        return 1;
    case 0x2590:
        shape.left = midx;
        PaintRect(&shape);
        return 1;
    case 0x2591:
    case 0x2592:
    case 0x2593:
        {
            Pattern pat;

#if TARGET_API_MAC_CARBON
            if (ch == 0x2591)
                GetQDGlobalsLightGray(&pat);
            else if (ch == 0x2593)
                GetQDGlobalsDarkGray(&pat);
            else
                GetQDGlobalsGray(&pat);
#else
            if (ch == 0x2591)
                pat = qd.ltGray;
            else if (ch == 0x2593)
                pat = qd.dkGray;
            else
                pat = qd.gray;
#endif
            PenPat(&pat);
            PaintRect(cell);
            PenNormal();
        }
        return 1;
    case 0x23BA:
    case 0x23BB:
    case 0x23BC:
    case 0x23BD:
        shape.top = cell->top + (int)(ch - 0x23BA) * (cell->bottom - cell->top - 1) / 3;
        MoveTo(cell->left, shape.top);
        LineTo(cell->right - 1, shape.top);
        return 1;
    case 0x25A0:
        InsetRect(&shape, (cell->right - cell->left) / 4, (cell->bottom - cell->top) / 3);
        PaintRect(&shape);
        return 1;
    case 0x25CB:
        SetRect(&shape, cell->left + 1, midy - (cell->right - cell->left) / 2 + 1,
                cell->right - 1, midy + (cell->right - cell->left) / 2 - 1);
        FrameOval(&shape);
        return 1;
    case 0x263A:
        draw_face(cell, 0, bg);
        return 1;
    case 0x263B:
        draw_face(cell, 1, bg);
        return 1;
    case 0x263C:
        SetRect(&shape, midx - 2, midy - 2, midx + 3, midy + 3);
        FrameOval(&shape);
        MoveTo(midx, midy - 5);
        LineTo(midx, midy + 5);
        MoveTo(cell->left, midy);
        LineTo(cell->right - 1, midy);
        return 1;
    case 0x2310:
        MoveTo(cell->right - 2, midy);
        LineTo(cell->left + 1, midy);
        LineTo(cell->left + 1, midy + (cell->bottom - cell->top) / 5);
        return 1;
    case 0x00B2:
        draw_small_pair(cell, '2', 0);
        return 1;
    case 0x207F:
        draw_small_pair(cell, 'n', 0);
        return 1;
    case 0x00BD:
        draw_small_pair(cell, '1', '2');
        return 1;
    case 0x00BC:
        draw_small_pair(cell, '1', '4');
        return 1;
    case 0x20A7:
        draw_small_pair(cell, 'P', 't');
        return 1;
    }
    return 0;
}

static unsigned char unicode_to_symbol(unsigned long ch)
{
    static const struct
    {
        unsigned short code;
        unsigned char symbol;
    } symbol_table[] =
    {
        { 0x2190, 0xAC }, { 0x2191, 0xAD },
        { 0x2192, 0xAE }, { 0x2193, 0xAF }, { 0x2219, 0xB7 }, { 0x2229, 0xC7 },
        { 0x2261, 0xBA }, { 0x2320, 0xF3 }, { 0x2321, 0xF5 }, { 0x25C6, 0xA8 },
        { 0x2660, 0xAA }, { 0x2663, 0xA7 }, { 0x2665, 0xA9 }, { 0x2666, 0xA8 }
    };
    static const char greek_upper[] = "ABGDEZHQIKLMNXOPR STUFCYW";
    static const char greek_lower[] = "abgdezhqiklmnxoprVstufcyw";
    static const char greek_accented_upper[] = "A EHI O UW";
    static const char greek_accented_lower[] = "aehi";
    static const char greek_accented_final[] = "iuouw";
    int i;

    if (ch >= 0x0386 && ch <= 0x038F && greek_accented_upper[ch - 0x0386] != ' ')
        return (unsigned char)greek_accented_upper[ch - 0x0386];
    if (ch >= 0x03AC && ch <= 0x03AF)
        return (unsigned char)greek_accented_lower[ch - 0x03AC];
    if (ch >= 0x03CA && ch <= 0x03CE)
        return (unsigned char)greek_accented_final[ch - 0x03CA];
    if (ch >= 0x0391 && ch <= 0x03A9 && ch != 0x03A2)
        return (unsigned char)greek_upper[ch - 0x0391];
    if (ch >= 0x03B1 && ch <= 0x03C9)
        return (unsigned char)greek_lower[ch - 0x03B1];
    for (i = 0; i < (int)(sizeof(symbol_table) / sizeof(symbol_table[0])); i++)
        if (symbol_table[i].code == ch)
            return symbol_table[i].symbol;
    return 0;
}

static void draw_symbol(unsigned char code, const Rect *cell)
{
    int cellwidth;
    int width;

    cellwidth = cell->right - cell->left;
    TextFont(kFontIDSymbol);
    width = CharWidth(code);
    if (width > cellwidth)
    {
        TextSize((short)(PDC_font_size * cellwidth / width));
        width = CharWidth(code);
    }
    MoveTo(cell->left + (cellwidth - width) / 2, cell->top + PDC_font_ascent);
    DrawChar(code);
    TextFont(monaco);
    TextSize(PDC_font_size);
}

static void draw_text_glyph(unsigned long ch, int unicode, const Rect *cell)
{
    unsigned char code;

    code = (unsigned char)ch;
    if (unicode && ch >= 0x80)
    {
        code = PDC_unicode_to_macroman(ch);
        if (code == '?')
        {
            unsigned char symbolcode;

            symbolcode = unicode_to_symbol(ch);
            if (symbolcode)
            {
                draw_symbol(symbolcode, cell);
                return;
            }
        }
    }
    if (code < 32)
        code = (unsigned char)' ';
    MoveTo(cell->left, cell->top + PDC_font_ascent);
    DrawChar(code);
}

#ifdef USING_COMBINING_CHARACTER_SCHEME
static unsigned char compose_macroman(unsigned long base, unsigned long mark)
{
    static const struct
    {
        unsigned char base;
        unsigned char mark;
        unsigned char macroman;
    } compose_table[] =
    {
        { 'A', 0x08, 0x80 }, { 'A', 0x0A, 0x81 }, { 'C', 0x27, 0x82 }, { 'E', 0x01, 0x83 },
        { 'N', 0x03, 0x84 }, { 'O', 0x08, 0x85 }, { 'U', 0x08, 0x86 }, { 'a', 0x01, 0x87 },
        { 'a', 0x00, 0x88 }, { 'a', 0x02, 0x89 }, { 'a', 0x08, 0x8A }, { 'a', 0x03, 0x8B },
        { 'a', 0x0A, 0x8C }, { 'c', 0x27, 0x8D }, { 'e', 0x01, 0x8E }, { 'e', 0x00, 0x8F },
        { 'e', 0x02, 0x90 }, { 'e', 0x08, 0x91 }, { 'i', 0x01, 0x92 }, { 'i', 0x00, 0x93 },
        { 'i', 0x02, 0x94 }, { 'i', 0x08, 0x95 }, { 'n', 0x03, 0x96 }, { 'o', 0x01, 0x97 },
        { 'o', 0x00, 0x98 }, { 'o', 0x02, 0x99 }, { 'o', 0x08, 0x9A }, { 'o', 0x03, 0x9B },
        { 'u', 0x01, 0x9C }, { 'u', 0x00, 0x9D }, { 'u', 0x02, 0x9E }, { 'u', 0x08, 0x9F },
        { 'A', 0x00, 0xCB }, { 'A', 0x03, 0xCC }, { 'O', 0x03, 0xCD }, { 'y', 0x08, 0xD8 },
        { 'Y', 0x08, 0xD9 }, { 'A', 0x02, 0xE5 }, { 'E', 0x02, 0xE6 }, { 'A', 0x01, 0xE7 },
        { 'E', 0x08, 0xE8 }, { 'E', 0x00, 0xE9 }, { 'I', 0x01, 0xEA }, { 'I', 0x02, 0xEB },
        { 'I', 0x08, 0xEC }, { 'I', 0x00, 0xED }, { 'O', 0x01, 0xEE }, { 'O', 0x02, 0xEF },
        { 'O', 0x00, 0xF1 }, { 'U', 0x01, 0xF2 }, { 'U', 0x02, 0xF3 }, { 'U', 0x00, 0xF4 }
    };
    int i;

    if (base > 0x7F || mark < 0x300 || mark > 0x327)
        return 0;
    for (i = 0; i < (int)(sizeof(compose_table) / sizeof(compose_table[0])); i++)
        if (compose_table[i].base == base && compose_table[i].mark == mark - 0x300)
            return compose_table[i].macroman;
    return 0;
}

static unsigned char combining_accent(unsigned long mark)
{
    switch (mark)
    {
    case 0x300:
        return 0x60;
    case 0x301:
        return 0xAB;
    case 0x302:
        return 0xF6;
    case 0x303:
        return 0xF7;
    case 0x304:
        return 0xF8;
    case 0x306:
        return 0xF9;
    case 0x307:
        return 0xFA;
    case 0x308:
        return 0xAC;
    case 0x30A:
        return 0xFB;
    case 0x30B:
        return 0xFD;
    case 0x30C:
        return 0xFF;
    case 0x327:
        return 0xFC;
    case 0x328:
        return 0xFE;
    }
    return 0;
}
#endif

static void draw_cursor_shape(Rect *cell, const RGBColor *fg)
{
    int style;
    Rect bar;

    style = SP->drawing_cursor;
    RGBForeColor(fg);
    bar = *cell;
    if (style == 2)
    {
        InvertRect(&bar);
        return;
    }
    if (style == 1)
    {
        bar.top = bar.bottom - PDC_font_height / 5;
        if (bar.top < cell->top)
            bar.top = cell->top;
        PaintRect(&bar);
    }
    else if (style == 5)
    {
        bar.top = (cell->top + cell->bottom) / 2;
        PaintRect(&bar);
    }
    else if (style == 3)
    {
        FrameRect(&bar);
    }
    else if (style == 4)
    {
        MoveTo(cell->left + 1, cell->top);
        LineTo(cell->left + 1, cell->bottom - 1);
    }
}

void PDC_mac_default_background(RGBColor *color)
{
    PACKED_RGB fgpacked;
    PACKED_RGB bgpacked;

    bgpacked = PACK_RGB(0, 0, 0);
    if (SP && SP->pairs_allocated > 0)
        PDC_get_rgb_values((chtype)0, &fgpacked, &bgpacked);
    if (bgpacked == (PACKED_RGB)-1)
        bgpacked = PACK_RGB(0, 0, 0);
    packed_to_rgb(bgpacked, color);
}

void PDC_transform_line(int lineno, int x, int len, const chtype *srcp)
{
    int i;
    Rect clipbounds;

    PDC_LOG(("PDC_transform_line() - called: lineno=%d\n", lineno));
    if (!srcp || len <= 0 || !PDC_window)
        return;
    PDC_mac_set_port();
    PDC_mac_port_bounds(&clipbounds);
    TextFont(monaco);
    TextSize(PDC_font_size);
    for (i = 0; i < len; i++)
    {
        chtype cell;
        unsigned long ch;
        PACKED_RGB fgpacked;
        PACKED_RGB bgpacked;
        RGBColor fg;
        RGBColor bg;
        Rect r;
        Style face;
        attr_t sysattrs;
        int unicode;
#ifdef USING_COMBINING_CHARACTER_SCHEME
        cchar_t marks[4];
        int markcount;
        int mark;
#endif

        cell = srcp[i];
        sysattrs = SP->termattrs;
#ifdef PDC_WIDE
        unicode = 1;
#else
        unicode = 0;
#endif
        if (_is_altcharset(cell))
        {
            ch = (unsigned long)acs_map[cell & 0x7f];
            unicode = 1;
        }
        else
            ch = (unsigned long)(cell & A_CHARTEXT);
#ifdef USING_COMBINING_CHARACTER_SCHEME
        markcount = 0;
        while (ch > MAX_UNICODE)
        {
            cchar_t added;

            ch = (unsigned long)PDC_expand_combined_characters((cchar_t)ch, &added);
            if (markcount < 4)
                marks[markcount++] = added;
        }
        if (ch == DUMMY_CHAR_NEXT_TO_FULLWIDTH)
            ch = (unsigned long)' ';
        if (markcount == 1 && compose_macroman(ch, (unsigned long)marks[0]))
        {
            ch = (unsigned long)PDC_macroman_to_unicode(compose_macroman(ch, (unsigned long)marks[0]));
            markcount = 0;
        }
#endif
        if ((cell & A_BLINK) && SP->blink_state && (sysattrs & A_BLINK))
            ch = (unsigned long)' ';
        PDC_get_rgb_values(cell, &fgpacked, &bgpacked);
        if (fgpacked == (PACKED_RGB)-1)
            fgpacked = PACK_RGB(0xC0, 0xC0, 0xC0);
        if (bgpacked == (PACKED_RGB)-1)
            bgpacked = PACK_RGB(0, 0, 0);
        packed_to_rgb(fgpacked, &fg);
        packed_to_rgb(bgpacked, &bg);
        PDC_mac_cell_rect(lineno, x + i, &r);
        ClipRect(&r);
        fill_rect(&r, &bg);
        RGBForeColor(&fg);
        RGBBackColor(&bg);
        face = 0;
        if ((cell & A_BOLD) && (sysattrs & A_BOLD))
            face |= bold;
        if ((cell & A_ITALIC) && (sysattrs & A_ITALIC))
            face |= italic;
        TextFace(face);
        if (!unicode || !draw_graphic(ch, &r, &fg, &bg))
        {
            RGBForeColor(&fg);
            draw_text_glyph(ch, unicode, &r);
        }
#ifdef USING_COMBINING_CHARACTER_SCHEME
        for (mark = markcount - 1; mark >= 0; mark--)
            if (combining_accent((unsigned long)marks[mark]))
            {
                MoveTo(r.left, r.top + PDC_font_ascent);
                DrawChar(combining_accent((unsigned long)marks[mark]));
            }
#endif
        if (cell & (A_UNDERLINE | A_TOP | A_LEFT | A_RIGHT | A_STRIKEOUT))
        {
            RGBColor linec;
            int lineidx;

            lineidx = SP->line_color;
            if (lineidx != -1)
                packed_to_rgb(PDC_get_palette_entry(lineidx), &linec);
            else
                linec = fg;
            RGBForeColor(&linec);
            PenNormal();
            if (cell & A_UNDERLINE)
            {
                MoveTo(r.left, r.bottom - 1);
                LineTo(r.right - 1, r.bottom - 1);
            }
            if (cell & A_TOP)
            {
                MoveTo(r.left, r.top);
                LineTo(r.right - 1, r.top);
            }
            if (cell & A_STRIKEOUT)
            {
                int midy;

                midy = (r.top + r.bottom) / 2;
                MoveTo(r.left, midy);
                LineTo(r.right - 1, midy);
            }
            if (cell & A_LEFT)
            {
                MoveTo(r.left, r.top);
                LineTo(r.left, r.bottom - 1);
            }
            if (cell & A_RIGHT)
            {
                MoveTo(r.right - 1, r.top);
                LineTo(r.right - 1, r.bottom - 1);
            }
        }
        if (SP->drawing_cursor)
            draw_cursor_shape(&r, &fg);
        ClipRect(&clipbounds);
    }
    TextFace(0);
#ifdef MAC_OS_X_VERSION_10_0
    {
        CGrafPtr port;

        port = GetWindowPort(PDC_window);
        if (port)
            QDFlushPortBuffer(port, NULL);
    }
#endif
}

void PDC_doupdate(void)
{
#ifdef MAC_OS_X_VERSION_10_0
    if (PDC_window)
    {
        CGrafPtr port;

        port = GetWindowPort(PDC_window);
        if (port)
            QDFlushPortBuffer(port, NULL);
    }
#endif
}
