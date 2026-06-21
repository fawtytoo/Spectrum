//  Copyright 2026      by Steve Clark

//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.

//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:

//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required. 
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.

#include "types.h"

#include "ula.h"

typedef struct
{
    int     width;
    int     height;
}
ULA;

//                             48k         128k
static ULA      ulaSize[2] = {{448, 312}, {456, 311}};
static ULA      *ulaCur = &ulaSize[0];

static BYTE     ulaBorder[2];

// external --------------------------------------------------------------------
short       ulaPin[48];
WORD        ulaAddress;
BYTE        ulaDataIn, ulaDataOut;
BYTE        ulaKeyData;
int         ulaAddState, ulaState;

int ULA_Type(int mode)
{
    ulaCur = &ulaSize[mode];
    ULA_SYNC |= SYNC_VERT;

    return ulaCur->width * ulaCur->height;
}

void ULA_Cycle()
{
    static int      hc = 0, vc = 0;
    static int      interrupt = 0;
    static int      psg[2] = {1, 0};

    static int      flash = 0;
    static BYTE     byte, out, attr;
    static int      colour[2];
    static WORD     address[2];
    static int      draw = 0;

    int             pos;
    int             pixel = 0;

    int             hc0 = hc & 1;

    ULA_C = psg[0] & psg[1]; // psg clock
    psg[hc0] ^= 1;

    ULA_PHICPU = hc0;

    ULA_RAS = 0;
    ULA_CAS = 0;
    ulaAddState = HIGH;

    if (hc < 256 && vc < 192)
    {
        switch (hc & 15)
        {
          case 0:
            pos = vc * 32 + hc / 8;
            address[0] = (ULA_VB << 15) | 0x4000 | (pos & 0x1800) | ((pos << 3) & 0x0700) | ((pos >> 3) & 0x00e0) | (pos & 0x001f);
            address[1] = (ULA_VB << 15) | 0x5800 | ((pos >> 3) & 0x0300);
            break;

          case 2:
            ulaAddress = address[0];
            ULA_RAS = 1;
            ulaAddState = LOW;
            break;

          case 3:
            ULA_CAS = 1;
            ulaAddState = LOW;
            break;

          case 4:
            byte = ulaDataIn;
            ulaAddress = address[1];
            break;

          case 5:
            ULA_CAS = 1;
            ulaAddState = LOW;
            break;

          case 6:
            attr = ulaDataIn;
            out = byte;
            colour[(attr & flash) >> 7] = attr & 120;
            colour[1 - ((attr & flash) >> 7)] = attr & 71;
            draw = 8;
            address[0]++;
            ulaAddress = address[0];
            ULA_RAS = 1;
            ulaAddState = LOW;
            break;

          case 7:
            ULA_CAS = 1;
            ulaAddState = LOW;
            break;

          case 8:
            byte = ulaDataIn;
            ulaAddress = address[1];
            break;

          case 9:
            ULA_CAS = 1;
            ulaAddState = LOW;
            break;

          case 10:
            attr = ulaDataIn;
            break;

          case 14:
            out = byte;
            colour[(attr & flash) >> 7] = attr & 120;
            colour[1 - ((attr & flash) >> 7)] = attr & 71;
            draw = 8;
            break;

          default:
            break;
        }

        if ((hc & 15) < 12)
        {
            if (ULA_A14 || ULA_IORQ)
            {
                ULA_PHICPU = 0;
                if (ULA_IORQ && !(ULA_RD || ULA_WR))
                {
                    ULA_PHICPU = hc0;
                }
            }
        }
        else if (ULA_A14)
        {
            ULA_CAS = 1;
        }
    }

    ULA_RAS |= (ULA_PHICPU & ULA_MREQ & ULA_A14);
    ULA_CAS |= (ULA_PHICPU & ULA_MREQ & ULA_A14);
    ULA_DRAMWE = ULA_PHICPU & ULA_MREQ & ULA_WR & ULA_A14;
    ULA_ROMS = (!ULA_A14) & (!ULA_A15) & ULA_RD;

    ulaDataOut = 0xff;
    ulaState = HIGH;
    if (ULA_IORQ)
    {
        if (ULA_RD)
        {
            ulaDataOut = 0b10100000 | (ULA_EAR << 6) | ulaKeyData;
            ulaState = LOW;
        }
        else if (ULA_WR)
        {
            ulaBorder[0] = ulaDataIn & 7;
            ULA_MIC = (((ulaDataIn & 16) << 1) | (ulaDataIn & 8)) >> 3;
        }
    }

    if (draw == 0)
    {
        if ((hc & 7) == 6)
        {
            ulaBorder[1] = ulaBorder[0];
        }
        pixel = ulaBorder[1];
    }
    else
    {
        draw--;
        pixel = colour[out >> 7];
        pixel = ((pixel >> 3) | (pixel & 7)) & 15;
        out <<= 1;
    }

    ULA_B = pixel & 1;
    ULA_R = (pixel >> 1) & 1;
    ULA_G = (pixel >> 2) & 1;
    ULA_BRIGHT = pixel >> 3;

    ULA_SYNC = 0;

    hc++;
    if (hc == ulaCur->width - 42)
    {
        ULA_SYNC = SYNC_HORZ;
        vc++;
        if (vc == 192 + 56)
        {
            ULA_SYNC |= SYNC_VERT;
            flash += 8;
        }
        else if (vc == ulaCur->height)
        {
            vc = 0;
        }
    }
    else if (hc == ulaCur->width)
    {
        hc = 0;
        if (vc == 192 + 56)
        {
            interrupt = 32;
        }
    }
    else if (interrupt > 0)
    {
        interrupt -= hc0;
    }

    ULA_INT = interrupt > 0;
}
