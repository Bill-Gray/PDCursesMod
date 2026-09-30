#include "pdcmac.h"
#include <stdlib.h>
#include <string.h>

static const unsigned short macroman_unicode[128] =
{
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1,
    0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
    0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3,
    0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
    0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
    0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211,
    0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
    0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB,
    0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
    0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1,
    0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
    0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
    0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7
};

unsigned short PDC_macroman_to_unicode(unsigned char ch)
{
    if (ch < 0x80)
        return (unsigned short)ch;
    return macroman_unicode[ch - 0x80];
}

unsigned char PDC_unicode_to_macroman(unsigned long code)
{
    int i;

    if (code < 0x80)
        return (unsigned char)code;
    for (i = 0; i < 128; i++)
    {
        if ((unsigned long)macroman_unicode[i] == code)
            return (unsigned char)(i + 0x80);
    }
    return (unsigned char)'?';
}

static int utf8_encode(unsigned long code, char *out)
{
    if (code < 0x80)
    {
        out[0] = (char)code;
        return 1;
    }
    if (code < 0x800)
    {
        out[0] = (char)(0xC0 | (code >> 6));
        out[1] = (char)(0x80 | (code & 0x3F));
        return 2;
    }
    if (code < 0x10000)
    {
        out[0] = (char)(0xE0 | (code >> 12));
        out[1] = (char)(0x80 | ((code >> 6) & 0x3F));
        out[2] = (char)(0x80 | (code & 0x3F));
        return 3;
    }
    out[0] = (char)(0xF0 | (code >> 18));
    out[1] = (char)(0x80 | ((code >> 12) & 0x3F));
    out[2] = (char)(0x80 | ((code >> 6) & 0x3F));
    out[3] = (char)(0x80 | (code & 0x3F));
    return 4;
}

static unsigned long utf8_decode(const unsigned char **pp, const unsigned char *end)
{
    const unsigned char *p;
    unsigned long code;
    unsigned int extra;

    p = *pp;
    if (p >= end)
        return 0;
    if (p[0] < 0x80)
    {
        code = p[0];
        *pp = p + 1;
        return code;
    }
    if ((p[0] & 0xE0) == 0xC0)
    {
        extra = 1;
        code = p[0] & 0x1F;
    }
    else if ((p[0] & 0xF0) == 0xE0)
    {
        extra = 2;
        code = p[0] & 0x0F;
    }
    else if ((p[0] & 0xF8) == 0xF0)
    {
        extra = 3;
        code = p[0] & 0x07;
    }
    else
    {
        *pp = p + 1;
        return (unsigned long)'?';
    }
    if (p + extra >= end)
    {
        *pp = end;
        return (unsigned long)'?';
    }
    while (extra)
    {
        p++;
        extra--;
        if ((p[0] & 0xC0) != 0x80)
        {
            *pp = p;
            return (unsigned long)'?';
        }
        code = (code << 6) | (p[0] & 0x3F);
    }
    *pp = p + 1;
    return code;
}

static int use_utf8_clipboard(void)
{
#ifdef PDC_WIDE
    return 1;
#else
#ifdef PDC_FORCE_UTF8
    return 1;
#else
    return 0;
#endif
#endif
}

static char *macroman_buffer_to_result(const char *src, long srclen, long *outlen)
{
    char *result;
    long i;
    long destlen;
    long destpos;

    if (!use_utf8_clipboard())
    {
        result = (char *)malloc((size_t)srclen + 1);
        if (!result)
            return NULL;
        memcpy(result, src, (size_t)srclen);
        result[srclen] = 0;
        *outlen = srclen;
        return result;
    }
    destlen = 0;
    for (i = 0; i < srclen; i++)
    {
        unsigned long code;
        char temp[4];

        code = PDC_macroman_to_unicode((unsigned char)src[i]);
        destlen += utf8_encode(code, temp);
    }
    result = (char *)malloc((size_t)destlen + 1);
    if (!result)
        return NULL;
    destpos = 0;
    for (i = 0; i < srclen; i++)
    {
        unsigned long code;
        char temp[4];
        int n;

        code = PDC_macroman_to_unicode((unsigned char)src[i]);
        n = utf8_encode(code, temp);
        memcpy(result + destpos, temp, (size_t)n);
        destpos += n;
    }
    result[destpos] = 0;
    *outlen = destpos;
    return result;
}

static char *result_to_macroman(const char *src, long srclen, long *outlen)
{
    char *result;
    long destpos;

    if (!use_utf8_clipboard())
    {
        result = (char *)malloc((size_t)srclen + 1);
        if (!result)
            return NULL;
        memcpy(result, src, (size_t)srclen);
        result[srclen] = 0;
        *outlen = srclen;
        return result;
    }
    result = (char *)malloc((size_t)srclen + 1);
    if (!result)
        return NULL;
    destpos = 0;
    {
        const unsigned char *p;
        const unsigned char *end;

        p = (const unsigned char *)src;
        end = p + srclen;
        while (p < end)
        {
            unsigned long code;

            code = utf8_decode(&p, end);
            result[destpos] = (char)PDC_unicode_to_macroman(code);
            destpos++;
        }
    }
    result[destpos] = 0;
    *outlen = destpos;
    return result;
}

int PDC_getclipboard(char **contents, long *length)
{
    char *raw;
    long rawlen;

    PDC_LOG(("PDC_getclipboard() - called\n"));
    *contents = NULL;
    *length = 0;
#if TARGET_API_MAC_CARBON
    {
        ScrapRef scrap;
        Size bytecount;
        OSStatus err;

        err = GetCurrentScrap(&scrap);
        if (err != noErr)
            return PDC_CLIP_ACCESS_ERROR;
        err = GetScrapFlavorSize(scrap, kScrapFlavorTypeText, &bytecount);
        if (err == noTypeErr || bytecount <= 0)
            return PDC_CLIP_EMPTY;
        if (err != noErr)
            return PDC_CLIP_ACCESS_ERROR;
        raw = (char *)malloc((size_t)bytecount + 1);
        if (!raw)
            return PDC_CLIP_MEMORY_ERROR;
        err = GetScrapFlavorData(scrap, kScrapFlavorTypeText, &bytecount, raw);
        if (err != noErr)
        {
            free(raw);
            return PDC_CLIP_ACCESS_ERROR;
        }
        rawlen = (long)bytecount;
        raw[rawlen] = 0;
    }
#else
    {
        Handle datahandle;
        long offset;
        long scraplen;

        datahandle = NewHandle(0);
        if (!datahandle)
            return PDC_CLIP_MEMORY_ERROR;
        scraplen = GetScrap(datahandle, kScrapFlavorTypeText, &offset);
        if (scraplen <= 0)
        {
            DisposeHandle(datahandle);
            if (scraplen == 0 || scraplen == noTypeErr)
                return PDC_CLIP_EMPTY;
            return PDC_CLIP_ACCESS_ERROR;
        }
        HLock(datahandle);
        raw = (char *)malloc((size_t)scraplen + 1);
        if (!raw)
        {
            HUnlock(datahandle);
            DisposeHandle(datahandle);
            return PDC_CLIP_MEMORY_ERROR;
        }
        BlockMoveData(*datahandle, raw, scraplen);
        rawlen = scraplen;
        raw[rawlen] = 0;
        HUnlock(datahandle);
        DisposeHandle(datahandle);
    }
#endif
    *contents = macroman_buffer_to_result(raw, rawlen, length);
    free(raw);
    if (!*contents)
        return PDC_CLIP_MEMORY_ERROR;
    return PDC_CLIP_SUCCESS;
}

int PDC_setclipboard(const char *contents, long length)
{
    char *macroman;
    long maclen;
#if TARGET_API_MAC_CARBON
    OSStatus err;
#else
    OSErr err;
#endif

    PDC_LOG(("PDC_setclipboard() - called\n"));
    if (!contents)
        return PDC_clearclipboard();
    macroman = result_to_macroman(contents, length, &maclen);
    if (!macroman)
        return PDC_CLIP_MEMORY_ERROR;
#if TARGET_API_MAC_CARBON
    {
        ScrapRef scrap;

        err = ClearCurrentScrap();
        if (err == noErr)
            err = GetCurrentScrap(&scrap);
        if (err == noErr)
            err = PutScrapFlavor(scrap, kScrapFlavorTypeText, 0, maclen, macroman);
    }
#else
    err = ZeroScrap();
    if (err == noErr)
        err = PutScrap(maclen, kScrapFlavorTypeText, macroman);
#endif
    free(macroman);
    if (err != noErr)
        return PDC_CLIP_ACCESS_ERROR;
    return PDC_CLIP_SUCCESS;
}

int PDC_freeclipboard(char *contents)
{
    PDC_LOG(("PDC_freeclipboard() - called\n"));
    if (contents)
        free(contents);
    return PDC_CLIP_SUCCESS;
}

int PDC_clearclipboard(void)
{
#if TARGET_API_MAC_CARBON
    OSStatus err;
#else
    OSErr err;
#endif

    PDC_LOG(("PDC_clearclipboard() - called\n"));
#if TARGET_API_MAC_CARBON
    err = ClearCurrentScrap();
#else
    err = ZeroScrap();
#endif
    if (err != noErr)
        return PDC_CLIP_ACCESS_ERROR;
    return PDC_CLIP_SUCCESS;
}
