#include "framebuffer.h"

#include "mailbox.h"
#include "uart.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define COLOR_DEPTH 32

unsigned int width, height, pitch, isrgb;
unsigned char *framebuffer;

// https://en.wikipedia.org/wiki/Enhanced_Graphics_Adapter
unsigned int vgapal[] = {0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
                         0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
                         0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
                         0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF};

// https://en.wikipedia.org/wiki/Code_page_437
unsigned char font[FONT_NUMGLYPHS][FONT_BPG] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0000 (nul)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0001
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0002
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0003
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0004
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0005
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0006
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0007
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0008
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0009
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+000A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+000B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+000C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+000D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+000E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+000F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0010
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0011
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0012
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0013
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0014
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0015
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0016
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0017
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0018
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0019
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+001A
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+001B
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+001C
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+001D
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+001E
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+001F
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0020 (space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},  // U+0021 (!)
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0022 (")
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},  // U+0023 (#)
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},  // U+0024 ($)
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},  // U+0025 (%)
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},  // U+0026 (&)
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0027 (')
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},  // U+0028 (()
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},  // U+0029 ())
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},  // U+002A (*)
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},  // U+002B (+)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},  // U+002C (,)
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},  // U+002D (-)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},  // U+002E (.)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},  // U+002F (/)
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},  // U+0030 (0)
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},  // U+0031 (1)
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},  // U+0032 (2)
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},  // U+0033 (3)
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},  // U+0034 (4)
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},  // U+0035 (5)
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},  // U+0036 (6)
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},  // U+0037 (7)
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},  // U+0038 (8)
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},  // U+0039 (9)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},  // U+003A (:)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},  // U+003B (;)
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},  // U+003C (<)
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},  // U+003D (=)
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},  // U+003E (>)
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},  // U+003F (?)
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},  // U+0040 (@)
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},  // U+0041 (A)
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},  // U+0042 (B)
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},  // U+0043 (C)
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},  // U+0044 (D)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},  // U+0045 (E)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},  // U+0046 (F)
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},  // U+0047 (G)
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},  // U+0048 (H)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // U+0049 (I)
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},  // U+004A (J)
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},  // U+004B (K)
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},  // U+004C (L)
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},  // U+004D (M)
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},  // U+004E (N)
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},  // U+004F (O)
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},  // U+0050 (P)
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},  // U+0051 (Q)
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},  // U+0052 (R)
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},  // U+0053 (S)
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // U+0054 (T)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},  // U+0055 (U)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},  // U+0056 (V)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},  // U+0057 (W)
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},  // U+0058 (X)
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},  // U+0059 (Y)
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},  // U+005A (Z)
    {0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},  // U+005B ([)
    {0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},  // U+005C (\)
    {0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},  // U+005D (])
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},  // U+005E (^)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},  // U+005F (_)
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+0060 (`)
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},  // U+0061 (a)
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},  // U+0062 (b)
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},  // U+0063 (c)
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},  // U+0064 (d)
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},  // U+0065 (e)
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},  // U+0066 (f)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},  // U+0067 (g)
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},  // U+0068 (h)
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // U+0069 (i)
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},  // U+006A (j)
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},  // U+006B (k)
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // U+006C (l)
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},  // U+006D (m)
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},  // U+006E (n)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},  // U+006F (o)
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},  // U+0070 (p)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},  // U+0071 (q)
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},  // U+0072 (r)
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},  // U+0073 (s)
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},  // U+0074 (t)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},  // U+0075 (u)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},  // U+0076 (v)
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},  // U+0077 (w)
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},  // U+0078 (x)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},  // U+0079 (y)
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},  // U+007A (z)
    {0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},  // U+007B ({)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},  // U+007C (|)
    {0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},  // U+007D (})
    {0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+007E (~)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // U+007F
    {0x1E, 0x33, 0x03, 0x33, 0x1E, 0x18, 0x30, 0x1E},  // U+00C7 (C cedille)
    {0x00, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},  // U+00FC (u umlaut)
    {0x38, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},  // U+00E9 (e aigu)
    {0x7E, 0xC3, 0x3C, 0x60, 0x7C, 0x66, 0xFC, 0x00},  // U+00E2 (a circumflex)
    {0x33, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},  // U+00E4 (a umlaut)
    {0x07, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},  // U+00E0 (a grave)
    {0x0C, 0x0C, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},  // U+00E5 (a ring)
    {0x00, 0x00, 0x1E, 0x03, 0x03, 0x1E, 0x30, 0x1C},  // U+00E7 (c cedille)
    {0x7E, 0xC3, 0x3C, 0x66, 0x7E, 0x06, 0x3C, 0x00},  // U+00EA (e circumflex)
    {0x33, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},  // U+00EB (e umlaut)
    {0x07, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},  // U+00E8 (e grave)
    {0x33, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // U+00EF (i umlaut)
    {0x3E, 0x63, 0x1C, 0x18, 0x18, 0x18, 0x3C, 0x00},  // U+00EE (i circumflex)
    {0x07, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // U+00EC (i grave)
    {0x63, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x63, 0x00},  // U+00C4 (A umlaut)
    {0x0C, 0x0C, 0x00, 0x1E, 0x33, 0x3F, 0x33, 0x00},  // U+00C5 (A ring)
    {0x07, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},  // U+00C8 (E grave)
    {0x00, 0x00, 0xFE, 0x30, 0xFE, 0x33, 0xFE, 0x00},  // U+00E6 (ae)
    {0x7C, 0x36, 0x33, 0x7F, 0x33, 0x33, 0x73, 0x00},  // U+00C6 (AE)
    {0x1E, 0x33, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},  // U+00F4 (o circumflex)
    {0x00, 0x33, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},  // U+00F6 (o umlaut)
    {0x00, 0x07, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},  // U+00F2 (o grave)
    {0x1E, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},  // U+00FB (u circumflex)
    {0x00, 0x07, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},  // U+00F9 (u grave)
    {0x00, 0x33, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F},  // U+00FF (y umlaut)
    {0xC3, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x18, 0x00},  // U+00D6 (O umlaut)
    {0x33, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00},  // U+00DC (U umlaut)
    {0x18, 0x18, 0x7E, 0x03, 0x03, 0x7E, 0x18, 0x18},  // U+00A2 (dollarcents)
    {0x1C, 0x36, 0x26, 0x0F, 0x06, 0x67, 0x3F,
     0x00},  // U+00A3 (pound sterling)
    {0x33, 0x33, 0x1E, 0x3F, 0x0C, 0x3F, 0x0C, 0x0C},  // U+00A5 (yen)
    {0x7C, 0xC6, 0x1C, 0x36, 0x36, 0x1C, 0x33, 0x1E},  // U+00A7 (paragraph)
    {0x70, 0xD8, 0x18, 0x3C, 0x18, 0x18, 0x1B, 0x0E},  // U+0192 (dutch florijn)
    {0x38, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},  // U+00E1 (a aigu)
    {0x1C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},  // U+00ED (i augu)
    {0x00, 0x38, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},  // U+00F3 (o aigu)
    {0x00, 0x38, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},  // U+00FA (u aigu)
    {0x00, 0x1F, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x00},  // U+00F1 (n ~)
    {0x3F, 0x00, 0x33, 0x37, 0x3F, 0x3B, 0x33, 0x00},  // U+00D1 (N ~)
    {0x3C, 0x36, 0x36, 0x7C, 0x00, 0x00, 0x00, 0x00},  // U+00AA (superscript a)
    {0x1C, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00},  // U+00BA (superscript 0)
    {0x0C, 0x00, 0x0C, 0x06, 0x03, 0x33, 0x1E, 0x00},  // U+00BF (inverted ?)
    {0x00, 0x00, 0x00, 0x3F, 0x03, 0x03, 0x00,
     0x00},  // U+2310 (gun pointing right)
    {0x00, 0x00, 0x00, 0x3F, 0x30, 0x30, 0x00,
     0x00},  // U+00AC (gun pointing left)
    {0xC3, 0x63, 0x33, 0x7B, 0xCC, 0x66, 0x33, 0xF0},  // U+00BD (1/2)
    {0xC3, 0x63, 0x33, 0xBD, 0xEC, 0xF6, 0xF3, 0x03},  // U+00BC (1/4)
    {0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00},  // U+00A1 (inverted !)
    {0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00},  // U+00AB (<<)
    {0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00},  // U+00BB (>>)
    {0x55, 0x00, 0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00},  // U+2591 (25% solid)
    {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA},  // U+2592 (50% solid)
    {0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA, 0xFF, 0x55},  // U+2593 (75% solid)
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},  // U+2502 (thin vertical)
    {0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08,
     0x08},  // U+2524 (down L, left L, up L)
    {0x08, 0x08, 0x08, 0x0F, 0x08, 0x0F, 0x08,
     0x08},  // U+2561 (up L, down L, left D)
    {0x14, 0x14, 0x14, 0x14, 0x17, 0x14, 0x14,
     0x14},  // U+2562 (up D, down D, left L)
    {0x00, 0x00, 0x00, 0x00, 0x1F, 0x14, 0x14,
     0x14},  // U+2556 (down D, left L)
    {0x00, 0x00, 0x00, 0x0F, 0x08, 0x0F, 0x08,
     0x08},  // U+2555 (down L, left D)
    {0x14, 0x14, 0x14, 0x17, 0x10, 0x17, 0x14,
     0x14},  // U+2563 (up D, down D, left D)
    {0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
     0x14},  // U+2551 (double vertical)
    {0x00, 0x00, 0x00, 0x1F, 0x10, 0x17, 0x14,
     0x14},  // U+2557 (down D, left D)
    {0x14, 0x14, 0x14, 0x17, 0x10, 0x1F, 0x00, 0x00},  // U+255D (up D, left D)
    {0x14, 0x14, 0x14, 0x14, 0x1F, 0x00, 0x00, 0x00},  // U+255C (up D, left L)
    {0x08, 0x08, 0x08, 0x0F, 0x08, 0x0F, 0x00, 0x00},  // U+255B (up L, left D)
    {0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08,
     0x08},  // U+2510 (down L, left L)
    {0x08, 0x08, 0x08, 0x08, 0xf8, 0x00, 0x00, 0x00},  // U+2514 (up L, right L)
    {0x08, 0x08, 0x08, 0x08, 0xff, 0x00, 0x00,
     0x00},  // U+2534 (up L, right L, left L)
    {0x00, 0x00, 0x00, 0x00, 0xff, 0x08, 0x08,
     0x08},  // U+252C (down L, right L, left L)
    {0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08,
     0x08},  // U+251C (down L, right L, up L)
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00,
     0x00},  // U+2500 (thin horizontal)
    {0x08, 0x08, 0x08, 0x08, 0xff, 0x08, 0x08,
     0x08},  // U+253C (up L, right L, left L, down L)
    {0x08, 0x08, 0x08, 0xF8, 0x08, 0xF8, 0x08,
     0x08},  // U+255E (up L, down L, right D)
    {0x14, 0x14, 0x14, 0x14, 0xF4, 0x14, 0x14,
     0x14},  // U+255F (up D, down D, right L)
    {0x14, 0x14, 0x14, 0xF4, 0x04, 0xFC, 0x00, 0x00},  // U+255A (up D, right D)
    {0x00, 0x00, 0x00, 0xFC, 0x04, 0xF4, 0x14,
     0x14},  // U+2554 (down D, right D)
    {0x14, 0x14, 0x14, 0xF7, 0x00, 0xFF, 0x00,
     0x00},  // U+2569 (left D, right D, up D)
    {0x00, 0x00, 0x00, 0xFF, 0x00, 0xF7, 0x14,
     0x14},  // U+2566 (left D, right D, down D)
    {0x14, 0x14, 0x14, 0xF4, 0x04, 0xF4, 0x14,
     0x14},  // U+2560 (up D, down D, right D)
    {0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00,
     0x00},  // U+2550 (double horizontal)
    {0x14, 0x14, 0x14, 0xF7, 0x00, 0xF7, 0x14,
     0x14},  // U+256C (left D, right D, down D, up D)
    {0x08, 0x08, 0x08, 0xFF, 0x00, 0xFF, 0x00,
     0x00},  // U+2567 (left D, right D, up L)
    {0x14, 0x14, 0x14, 0x14, 0xFF, 0x00, 0x00,
     0x00},  // U+2568 (left L, right L, up D)
    {0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x08,
     0x08},  // U+2564 (left D, right D, down L)
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0x14, 0x14,
     0x14},  // U+2565 (left L, right L, down D)
    {0x14, 0x14, 0x14, 0x14, 0xFC, 0x00, 0x00, 0x00},  // U+2559 (up D, right L)
    {0x08, 0x08, 0x08, 0xF8, 0x08, 0xF8, 0x00, 0x00},  // U+2558 (up L, right D)
    {0x00, 0x00, 0x00, 0xF8, 0x08, 0xF8, 0x08,
     0x08},  // U+2552 (down L, right D)
    {0x00, 0x00, 0x00, 0x00, 0xFC, 0x14, 0x14,
     0x14},  // U+2553 (down D, right L)
    {0x14, 0x14, 0x14, 0x14, 0xFF, 0x14, 0x14,
     0x14},  // U+256B (left L, right L, down D, up D)
    {0x08, 0x08, 0x08, 0xFF, 0x08, 0xFF, 0x08,
     0x08},  // U+256A (left D, right D, down L, up L)
    {0x08, 0x08, 0x08, 0x08, 0x0f, 0x00, 0x00, 0x00},  // U+2518 (up L, left L)
    {0x00, 0x00, 0x00, 0x00, 0xf8, 0x08, 0x08,
     0x08},  // U+250C (down L, right L)
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},  // U+2588 (solid)
    {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},  // U+2584 (bottom half)
    {0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F},  // U+258C (left half)
    {0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0},  // U+2590 (right half)
    {0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00},  // U+2580 (top half)
};

// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#frame-buffer
void init_framebuffer() {
  // buffer size in bytes (including the header values, the end tag and padding)
  mailbox[0] = MAILBOX_BUFFER_SIZE;
  // buffer request/response code
  //   Request codes:
  //     0x00000000: process request
  mailbox[1] = MAILBOX_REQUEST_CODE;
  // Set physical display width/height
  // Tag: 0x00048003
  // Request:
  //   Length: 8
  //   Value:
  //     u32: width in pixels
  //     u32: height in pixels
  mailbox[2] = MAILBOX_TAG_SET_PHSICAL_DISPLAY;
  mailbox[3] = 8;
  mailbox[4] = 8;
  mailbox[5] = SCREEN_WIDTH;
  mailbox[6] = SCREEN_HEIGHT;
  // Set virtual (buffer) width/height
  // Tag: 0x00048004
  // Request
  //   Length: 8
  //   Value:
  //     u32: width in pixels
  //     u32: height in pixels
  mailbox[7] = MAILBOX_TAG_SET_VIRTUAL_BUFFER;
  mailbox[8] = 8;
  mailbox[9] = 8;
  mailbox[10] = SCREEN_WIDTH;
  mailbox[11] = SCREEN_HEIGHT;
  // Set virtual offset
  // Tag: 0x00048009
  // Request:
  //   Length: 8
  //   Value:
  //     u32: X in pixels
  //     u32: Y in pixels
  mailbox[12] = MAILBOX_TAG_SET_VIRTUAL_OFFSET;
  mailbox[13] = 8;
  mailbox[14] = 8;
  mailbox[15] = 0;
  mailbox[16] = 0;
  // Set depth
  // Tag: 0x00048005
  // Request:
  //   Length: 4
  //   Value:
  //     u32: bits per pixel
  mailbox[17] = MAILBOX_TAG_SET_DEPTH;
  mailbox[18] = 4;
  mailbox[19] = 4;
  mailbox[20] = COLOR_DEPTH;
  // Set pixel order
  // Tag: 0x00048006
  // Request:
  //   Length: 4
  //   Value:
  //     u32: state
  //   State:
  //     0x0: BGR
  //     0x1: RGB
  mailbox[21] = MAILBOX_TAG_SET_PIXEL_ORDER;
  mailbox[22] = 4;
  mailbox[23] = 4;
  mailbox[24] = 1;
  // Allocate buffer
  // Tag: 0x00040001
  // Response:
  //   Length: 8
  //   Value:
  //     u32: frame buffer base address in bytes
  //     u32: frame buffer size in bytes
  mailbox[25] = MAILBOX_TAG_ALLOCATE_BUFFER;
  mailbox[26] = 8;
  mailbox[27] = 8;
  mailbox[28] = 4096;
  mailbox[29] = 0;
  // Get pitch
  // Tag: 0x00040008
  // Response:
  //   Length: 4
  //   Value:
  //     u32: bytes per line
  mailbox[30] = MAILBOX_TAG_GET_PITCH;
  mailbox[31] = 4;
  mailbox[32] = 4;
  mailbox[33] = 0;

  mailbox[34] = MAILBOX_TAG_LAST;

  if (mailbox_call(MAILBOX_CHANNEL_PROP) && mailbox[20] == 32 &&
      mailbox[28] != 0) {
    // convert GPU address to ARM address
    mailbox[28] &= 0x3FFFFFFF;
    width = mailbox[5];
    height = mailbox[6];
    pitch = mailbox[33];
    framebuffer = (void *)((unsigned long)mailbox[28]);
  } else {
    puts("Error setting screen resolution to 1920x1080\n");
  }
}

void draw_raw_pixel(int x, int y, unsigned int val) {
  int offset = y * pitch + x * 4;
  *((unsigned int *)(framebuffer + offset)) = val;
}

void draw_pixel(int x, int y, color_t color) {
  int offset = y * pitch + x * 4;
  *((unsigned int *)(framebuffer + offset)) = vgapal[color];
}

void draw_rect(int x1, int y1, int x2, int y2, color_t color, int fill) {
  for (int y = y1; y <= y2; y++) {
    for (int x = x1; x <= x2; x++) {
      if ((x == x1 || x == x2) || (y == y1 || y == y2)) {
        draw_pixel(x, y, color);
      } else if (fill) {
        draw_pixel(x, y, color);
      } else if (!fill) {
        draw_pixel(x, y, 0);
      }
    }
  }
}

void clear_screen() { draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK, 1); }

void draw_line(int x1, int y1, int x2, int y2, color_t color) {
  int dx = x2 - x1;
  int dy = y2 - y1;
  int y = y1;
  int p = 2 * dy - dx;
  for (int x = x1; x < x2; x++) {
    draw_pixel(x, y, color);
    if (p >= 0) {
      y++;
      p += 2 * dy - 2 * dx;
    } else {
      p += 2 * dy;
    }
  }
}

void draw_circle(int x0, int y0, int radius, color_t color, int fill) {
  int x = radius;
  int y = 0;
  int err = 0;

  while (x >= y) {
    if (fill) {
      draw_line(x0 - y, y0 + x, x0 + y, y0 + x, color);
      draw_line(x0 - x, y0 + y, x0 + x, y0 + y, color);
      draw_line(x0 - x, y0 - y, x0 + x, y0 - y, color);
      draw_line(x0 - y, y0 - x, x0 + y, y0 - x, color);
    }
    draw_pixel(x0 - y, y0 + x, color);
    draw_pixel(x0 + y, y0 + x, color);
    draw_pixel(x0 - x, y0 + y, color);
    draw_pixel(x0 + x, y0 + y, color);
    draw_pixel(x0 - x, y0 - y, color);
    draw_pixel(x0 + x, y0 - y, color);
    draw_pixel(x0 - y, y0 - x, color);
    draw_pixel(x0 + y, y0 - x, color);

    if (err <= 0) {
      y += 1;
      err += 2 * y + 1;
    }

    if (err > 0) {
      x -= 1;
      err -= 2 * x + 1;
    }
  }
}

void draw_char(unsigned char ch, int x, int y, color_t color) {
  unsigned char *glyph =
      (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

  for (int i = 0; i < FONT_HEIGHT; i++) {
    for (int j = 0; j < FONT_WIDTH; j++) {
      unsigned char mask = 1 << j;
      unsigned char col = (*glyph & mask) ? color : BLACK;
      draw_pixel(x + j, y + i, col);
    }
    glyph += FONT_BPL;
  }
}

void draw_string(int x, int y, char *str, color_t color) {
  for (char *c = str; *c; c++) {
    if (*c == '\r') {
      x = 0;
    } else if (*c == '\n') {
      x = 0;
      y += FONT_HEIGHT;
    } else {
      draw_char(*c, x, y, color);
      x += FONT_WIDTH;
    }
  }
}

// draw max 4 digit number
void draw_number(int num, int x, int y, int color) {
  // draw the thousands digit
  draw_char(num / 1000 + '0', x, y, color);
  // draw the hundreds digit
  draw_char((num % 1000) / 100 + '0', x + FONT_WIDTH, y, color);
  // draw the tens digit
  draw_char((num % 100) / 10 + '0', x + 2 * FONT_WIDTH, y, color);
  // draw the ones digit
  draw_char(num % 10 + '0', x + 3 * FONT_WIDTH, y, color);
}