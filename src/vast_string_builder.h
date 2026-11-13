#ifndef VAST_STRING_BUILDER_H
#define VAST_STRING_BUILDER_H

/* #############################################################################
 * # [SECTION] Dependencies
 * #############################################################################
 */
#include "vast_types.h" /* Basic types (u8, u32, ...)    */

/* #############################################################################
 * # [SECTION] String Builder
 * #############################################################################
 */
typedef enum vast_sb_pad
{
    VAST_SB_PAD_LEFT,
    VAST_SB_PAD_RIGHT
} vast_sb_pad;

typedef struct vast_sb
{
    u32 size;
    u32 length;
    s8 *buffer;

} vast_sb;

VAST_API void vast_sb_write_raw(vast_sb *sb, s8 *s, u32 total_width, s8 pad_char, vast_sb_pad side)
{
    u32 s_len = 0;
    u32 pads;

    while (s[s_len])
    {
        s_len++;
    }

    pads = (total_width > s_len) ? (total_width - s_len) : 0;

    if (sb->length + s_len + pads >= sb->size)
    {
        return;
    }

    if (side == VAST_SB_PAD_LEFT)
    {
        while (pads--)
        {
            sb->buffer[sb->length++] = pad_char;
        }
    }

    while (*s)
    {
        sb->buffer[sb->length++] = *s++;
    }

    if (side == VAST_SB_PAD_RIGHT)
    {
        while (pads--)
        {
            sb->buffer[sb->length++] = pad_char;
        }
    }

    sb->buffer[sb->length] = 0;
}

VAST_API void vast_sb_s8(vast_sb *sb, s8 *s)
{
    vast_sb_write_raw(sb, s, 0, 0, VAST_SB_PAD_RIGHT);
}

VAST_API void vast_sb_s8_pad(vast_sb *sb, s8 *s, u32 width, s8 p, vast_sb_pad side)
{
    vast_sb_write_raw(sb, s, width, p, side);
}

VAST_API void vast_sb_i32_pad(vast_sb *sb, i32 v, u32 width, s8 p, vast_sb_pad side)
{
    s8 buf[12], *ptr = buf + 11;
    u32 u = (v < 0) ? (u32)-v : (u32)v;

    *ptr = 0;

    do
    {
        *--ptr = (s8)('0' + (u % 10));
        u /= 10;
    } while (u);

    if (v < 0)
    {
        *--ptr = '-';
    }

    vast_sb_write_raw(sb, ptr, width, p, side);
}

VAST_API void vast_sb_i32(vast_sb *sb, i32 v)
{
    vast_sb_i32_pad(sb, v, 0, 0, 0);
}

VAST_API void vast_sb_f64_pad(vast_sb *sb, f64 v, i32 dec, u32 width, s8 p, vast_sb_pad side)
{
    s8 buf[64];

    vast_sb tmp;
    tmp.size = 64;
    tmp.length = 0;
    tmp.buffer = buf;

    if (v < 0)
    {
        buf[tmp.length++] = '-';
        v = -v;
    }

    vast_sb_i32_pad(&tmp, (i32)v, 0, 0, 0);

    if (dec > 0)
    {
        f64 frac;
        i32 i;

        buf[tmp.length++] = '.';
        frac = v - (f64)((i32)v);

        for (i = 0; i < dec; ++i)
        {
            frac *= 10.0;
            buf[tmp.length++] = (s8)('0' + (i32)frac);
            frac -= (i32)frac;
        }
    }
    buf[tmp.length] = 0;
    vast_sb_write_raw(sb, buf, width, p, side);
}

VAST_API void vast_sb_f64(vast_sb *sb, f64 v, i32 dec)
{
    vast_sb_f64_pad(sb, v, dec, 0, 0, 0);
}

#endif /* VAST_STRING_BUILDER_H */