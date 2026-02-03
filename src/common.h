#ifndef __COMMON__
#define __COMMON__

#include "types.h"

// memory ----------------------------------------------------------------------
extern WORD     romAddress;
extern BYTE     romData;

void ROM_Cycle(void);

extern WORD     memAddress;
extern BYTE     memDataIn, memDataOut;
extern int      memState;
extern int      memBankIndex;

void MEM_Cycle(void);

void HAL_Cycle(void);

void MMU_Cycle(void);

extern WORD     pcfAddressIn;
extern WORD     pcfAddressOut[2];

void PCF_Cycle(void);

// CPU -------------------------------------------------------------------------
extern WORD     cpuAddress;
extern BYTE     cpuData;

void CPU_Cycle(void);
void ROM_LdBytes(void);
void ROM_SaContrl(void);

// ULA -------------------------------------------------------------------------
extern WORD     ulaAddress;
extern BYTE     ulaDataIn, ulaDataOut;
extern BYTE     ulaKeyData;
extern int      ulaState;

void ULA_Cycle(void);
void ULA_Read(void);
void ULA_Write(BYTE);
int ULA_Type(int);

// PSG -------------------------------------------------------------------------
extern BYTE     psgDataOut;
extern int      psgState;

void PSG_Read(void);
void PSG_Write(BYTE);
void PSG_Cycle(void);

// tape ------------------------------------------------------------------------
short TAPE_Input(short);

// joysticks -------------------------------------------------------------------
void Sinclair_Input(int, int, int);
void Sinclair_Read(void);

void Cursor_Input(int, int, int);
void Cursor_Read(void);

void Kempston_Input(int, int, int);
void Kempston_Read(void);

void Fuller_Input(int, int, int);
void Fuller_Read(void);

#endif
