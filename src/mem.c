#include "types.h"

#include "mem.h"

// memory ----------------------------------------------------------------------
#define M_16K       (1 << 14)

// rom -------------------------------------------------------------------------
static BYTE     romBank[2][M_16K];

WORD            romAddress;
BYTE            romData;

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

int             memBankIndex = 0;
WORD            memAddress;
BYTE            memDataIn, memDataOut;
int             memState;

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
    romData = romBank[ROM_A14][romAddress];
}

void ROM_Load(BYTE *data, int size)
{
    BYTE    *rom = romBank[0];

    while (size--)
    {
        *rom++ = *data++;
    }
}

// memory ----------------------------------------------------------------------
void MEM_Cycle()
{
    BANK    *bank = &memBank[memBankIndex];

    memDataOut = 0xff;
    memState = HIGH;

    if (MEM_RAS)
    {
        bank->row = memAddress & 0x00ff;
    }
    if (MEM_CAS)
    {
        bank->column = memAddress & 0xff00;
        if (MEM_WE)
        {
            *(bank->bank + bank->column + bank->row) = memDataIn;
        }
        else
        {
            memDataOut = *(bank->bank + bank->column + bank->row);
            memState = LOW;
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

// helper function for fast loading/saving -------------------------------------
void MEM_Direct(WORD address, BYTE *data, int write)
{
    int     index;
    int     a15 = address >> 15;
    int     a14 = (address >> 14) & 1;

    address &= 0x3fff;

    if (!(a15 | a14))
    {
        *data = romBank[ROM_A14][address];
        return;
    }

    index = ((!a15) & a14) | (a15 & a14 & HAL_B0);

    if (index == 0)     // uncontended
    {
        address |= ((a15 & a14 & (!HAL_B0) & HAL_B2) << 14);
        address |= (((a15 & (!a14)) | (a15 & a14 & (!HAL_B0) & HAL_B1)) << 15);
    }
    else                // contended
    {
        address |= ((((!a15) & a14) | (a15 & a14 & HAL_B0 & HAL_B2)) << 14);
        address |= ((a15 & a14 & HAL_B0 & HAL_B1) << 15);
    }

    if (write)
    {
        memBank[index].bank[address] = *data;
    }
    else
    {
        *data = memBank[index].bank[address];
    }
}
