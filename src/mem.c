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
#include "rom.h"

#include "mem.h"

// memory ----------------------------------------------------------------------
#define M_16K       (1 << 14)

// rom -------------------------------------------------------------------------
static BYTE     romBank[2][M_16K] = {{ROM1}, {ROM2}};

WORD            romAddress;
BYTE            romData;
int             romState;

short           romPin[28];

// -----------------------------------------------------------------------------
typedef struct
{
    BYTE    bank[M_16K * 4];
    WORD    row;
    WORD    column;
}
BANK;

static BANK     memBank[2];

WORD            memAddress[2];
BYTE            memDataIn, memDataOut[2];
int             memState[2];

short           memPin[16];

// mapping ---------------------------------------------------------------------
short       mmuPin[16];

// HAL10H8CN -------------------------------------------------------------------
short       halPin[20];

// PCF1306P --------------------------------------------------------------------
short       pcfPin[40];
WORD        pcfAddressIn;
WORD        pcfAddressOut[2];

// rom -------------------------------------------------------------------------
void ROM_Cycle()
{
    romData = 0xff;
    romState = HIGH;

    if (ROM_CE && ROM_OE)
    {
        romData = romBank[ROM_A14][romAddress];
        romState = LOW;
    }
}

// memory ----------------------------------------------------------------------
void MEM_Cycle(int index)
{
    BANK    *bank = &memBank[index];

    memDataOut[index] = 0xff;
    memState[index] = HIGH;

    if (MEM_RAS)
    {
        bank->row = memAddress[index] & 0x00ff;
    }
    if (MEM_CAS)
    {
        bank->column = memAddress[index] & 0xff00;
        if (MEM_WE)
        {
            *(bank->bank + bank->column + bank->row) = memDataIn;
        }
        else
        {
            memDataOut[index] = *(bank->bank + bank->column + bank->row);
            memState[index] = LOW;
        }
    }
}

// mapping ---------------------------------------------------------------------
void MMU_Cycle()
{
    MMU_Q0 = (!MMU_CLR) & MMU_D0;   // bank
    MMU_Q1 = (!MMU_CLR) & MMU_D1;   // page bit 0
    MMU_Q2 = (!MMU_CLR) & MMU_D2;   // page bit 1
    MMU_Q3 = (!MMU_CLR) & MMU_D3;   // video buffer
    MMU_Q4 = (!MMU_CLR) & MMU_D4;   // rom chip select
    MMU_Q5 = (!MMU_CLR) & MMU_D5;   // disable mmu
}

// HAL10H8CN -------------------------------------------------------------------
void HAL_Cycle()
{
    HAL_PSG = (HAL_WR & (!HAL_A1) & HAL_IORQ & HAL_A15) | (HAL_RD & (!HAL_A1) & HAL_IORQ & HAL_A15);

    // this has been fixed to prevent reading port 0x7ffd
    HAL_BANK = HAL_WR & (!HAL_RD) & (!HAL_A1) & HAL_IORQ & (!HAL_A15);

    HAL_ULA14 = ((!HAL_A15) & HAL_A14) | (HAL_A15 & HAL_A14 & HAL_B0);
    HAL_ULA15 = (HAL_A15 & (!HAL_A14)) | (HAL_A15 & HAL_A14 & (!HAL_B0)) | HAL_RFSH;

    HAL_UA14 = HAL_A15 & HAL_A14 & (!HAL_B0) & HAL_B2;
    HAL_UA15 = (HAL_A15 & (!HAL_A14)) | (HAL_A15 & HAL_A14 & (!HAL_B0) & HAL_B1);

    HAL_VA14 = ((!HAL_A15) & HAL_A14) | (HAL_A15 & HAL_A14 & HAL_B0 & HAL_B2);
    HAL_VA15 = HAL_A15 & HAL_A14 & HAL_B0 & HAL_B1;
}

// PCF1306P --------------------------------------------------------------------
void PCF_Cycle()
{
    PCF_CAS = (PCF_RD | PCF_WR) & PCF_A15 & PCF_MREQ;

    pcfAddressOut[0] = (PCF_TS1 << 15) | (PCF_TS2 << 14) | pcfAddressIn;
    pcfAddressOut[1] = pcfAddressIn;
}
