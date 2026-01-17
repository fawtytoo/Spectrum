#ifndef __MEM__
#define __MEM__

// rom -------------------------------------------------------------------------
#define ROMPIN_VPP      1
#define ROMPIN_A12      2   // <-
#define ROMPIN_A7       3   // <-
#define ROMPIN_A6       4   // <-
#define ROMPIN_A5       5   // <-
#define ROMPIN_A4       6   // <-
#define ROMPIN_A3       7   // <-
#define ROMPIN_A2       8   // <-
#define ROMPIN_A1       9   // <-
#define ROMPIN_A0       10  // <-
#define ROMPIN_D0       11  // <-
#define ROMPIN_D1       12  // <-
#define ROMPIN_D2       13  // <-
#define ROMPIN_GND      14
#define ROMPIN_D3       15  // <-
#define ROMPIN_D4       16  // <-
#define ROMPIN_D5       17  // <-
#define ROMPIN_D6       18  // <-
#define ROMPIN_D7       19  // <-
#define ROMPIN_CE       20  // <-   ULA_ROMS
#define ROMPIN_A10      21  // <-
#define ROMPIN_OE       22  // <-   CPU_MREQ
#define ROMPIN_A11      23  // <-
#define ROMPIN_A9       24  // <-
#define ROMPIN_A8       25  // <-
#define ROMPIN_A13      26  // <-
#define ROMPIN_A14      27  // <-   MMU_Q4
#define ROMPIN_VCC      28

#define ROM_CE          romPin[ROMPIN_CE - 1]
#define ROM_OE          romPin[ROMPIN_OE - 1]
#define ROM_A14         romPin[ROMPIN_A14 - 1]

extern short            romPin[28];

// MEMORY ----------------------------------------------------------------------
#define MEMPIN_NC       1
#define MEMPIN_DIN      2   // <-
#define MEMPIN_WE       3   // <-   CPU_WR or ULA_DRAMWE
#define MEMPIN_RAS      4   // <-   CPU_MREQ or ULA_RAS
#define MEMPIN_A0       5   // <-
#define MEMPIN_A2       6   // <-
#define MEMPIN_A1       7   // <-
#define MEMPIN_VCC      8
#define MEMPIN_A7       9   // <-
#define MEMPIN_A5       10  // <-
#define MEMPIN_A4       11  // <-
#define MEMPIN_A3       12  // <-
#define MEMPIN_A6       13  // <-
#define MEMPIN_DOUT     14  //  ->
#define MEMPIN_CAS      15  // <-   PCF_CAS or ULA_CAS
#define MEMPIN_GND      16

#define MEM_WE          memPin[MEMPIN_WE - 1]
#define MEM_RAS         memPin[MEMPIN_RAS - 1]
#define MEM_CAS         memPin[MEMPIN_CAS - 1]

extern short            memPin[16];

// 74LS174 ---------------------------------------------------------------------
#define MMUPIN_CLR      1   // <-   reset Q outputs
#define MMUPIN_Q0       2   //  ->  HAL_B0
#define MMUPIN_D0       3   // <-
#define MMUPIN_D1       4   // <-
#define MMUPIN_Q1       5   //  ->  HAL_B1
#define MMUPIN_D2       6   // <-
#define MMUPIN_Q2       7   //  ->  HAL_B2
#define MMUPIN_GND      8
#define MMUPIN_CP       9   // <-   HAL_BANK & (!MMU_Q5)
#define MMUPIN_Q3       10  //  ->  ULA_VB
#define MMUPIN_D3       11  // <-
#define MMUPIN_Q4       12  //  ->  ROM_A14
#define MMUPIN_D4       13  // <-
#define MMUPIN_D5       14  // <-
#define MMUPIN_Q5       15  //  ->  MMU_CP (disable)
#define MMUPIN_VCC      16

#define MMU_CLR         mmuPin[MMUPIN_CLR - 1]
#define MMU_Q0          mmuPin[MMUPIN_Q0 - 1]
#define MMU_D0          mmuPin[MMUPIN_D0 - 1]
#define MMU_D1          mmuPin[MMUPIN_D1 - 1]
#define MMU_Q1          mmuPin[MMUPIN_Q1 - 1]
#define MMU_D2          mmuPin[MMUPIN_D2 - 1]
#define MMU_Q2          mmuPin[MMUPIN_Q2 - 1]
#define MMU_CP          mmuPin[MMUPIN_CP - 1]
#define MMU_Q3          mmuPin[MMUPIN_Q3 - 1]
#define MMU_D3          mmuPin[MMUPIN_D3 - 1]
#define MMU_Q4          mmuPin[MMUPIN_Q4 - 1]
#define MMU_D4          mmuPin[MMUPIN_D4 - 1]
#define MMU_D5          mmuPin[MMUPIN_D5 - 1]
#define MMU_Q5          mmuPin[MMUPIN_Q5 - 1]

extern short            mmuPin[16];

// HAL10H8CN -------------------------------------------------------------------
#define HALPIN_A15      1   // <-   CPU_A15
#define HALPIN_A14      2   // <-   CPU_A14
#define HALPIN_A1       3   // <-   CPU_A1
#define HALPIN_IORQ     4   // <-   CPU_IOREQ
#define HALPIN_B0       5   // <-   MMU_Q0
#define HALPIN_B1       6   // <-   MMU_Q1
#define HALPIN_B2       7   // <-   MMU_Q2
#define HALPIN_RD       8   // <-   CPU_RD
#define HALPIN_WR       9   // <-   CPU_WR
#define HALPIN_GND      10
#define HALPIN_RFSH     11  // <-   CPU_RFSH (not connected causes snow)
#define HALPIN_PSG      12  //  ->  psg control
#define HALPIN_BANK     13  //  ->  MMU_CP
#define HALPIN_UA14     14  //  ->  PCF_TS2
#define HALPIN_UA15     15  //  ->  PCF_TS1
#define HALPIN_ULA14    16  //  ->  ULA_A14
#define HALPIN_ULA15    17  //  ->  ULA_A15 and PCF_A15
#define HALPIN_VA15     18  //  ->  MEM_A7 (contended memory)
#define HALPIN_VA14     19  //  ->  MEM_A7 (contended memory)
#define HALPIN_VCC      20

#define HAL_A15         halPin[HALPIN_A15 - 1]
#define HAL_A14         halPin[HALPIN_A14 - 1]
#define HAL_A1          halPin[HALPIN_A1 - 1]
#define HAL_IORQ        halPin[HALPIN_IORQ - 1]
#define HAL_B0          halPin[HALPIN_B0 - 1]
#define HAL_B1          halPin[HALPIN_B1 - 1]
#define HAL_B2          halPin[HALPIN_B2 - 1]
#define HAL_RD          halPin[HALPIN_RD - 1]
#define HAL_WR          halPin[HALPIN_WR - 1]
#define HAL_RFSH        halPin[HALPIN_RFSH - 1]
#define HAL_PSG         halPin[HALPIN_PSG - 1]
#define HAL_BANK        halPin[HALPIN_BANK - 1]
#define HAL_UA14        halPin[HALPIN_UA14 - 1]
#define HAL_UA15        halPin[HALPIN_UA15 - 1]
#define HAL_ULA14       halPin[HALPIN_ULA14 - 1]
#define HAL_ULA15       halPin[HALPIN_ULA15 - 1]
#define HAL_VA15        halPin[HALPIN_VA15 - 1]
#define HAL_VA14        halPin[HALPIN_VA14 - 1]

extern short            halPin[20];

// PCF1306P --------------------------------------------------------------------
#define PCFPIN_RAS      1   // <-   ULA_RAS
#define PCFPIN_A0       2   // <-   CPU_A0
#define PCFPIN_A7       3   // <-   CPU_A7
#define PCFPIN_V0       4   //  ->  MEM_A0 (contended memory)
#define PCFPIN_A1       5   // <-   CPU_A1
#define PCFPIN_A8       6   // <-   CPU_A8
#define PCFPIN_V1       7   //  ->  MEM_A1 (contended memory)
#define PCFPIN_A2       8   // <-   CPU_A2
#define PCFPIN_A9       9   // <-   CPU_A9
#define PCFPIN_V2       10  //  ->  MEM_A2 (contended memory)
#define PCFPIN_A3       11  // <-   CPU_A3
#define PCFPIN_A10      12  // <-   CPU_A10
#define PCFPIN_V3       13  //  ->  MEM_A3 (contended memory)
#define PCFPIN_A4       14  // <-   CPU_A4
#define PCFPIN_A11      15  // <-   CPU_A11
#define PCFPIN_V4       16  //  ->  MEM_A4 (contended memory)
#define PCFPIN_A5       17  // <-   CPU_A5
#define PCFPIN_A12      18  // <-   CPU_A12
#define PCFPIN_V5       19  //  ->  MEM_A5 (contended memory)
#define PCFPIN_GND      20
#define PCFPIN_A6       21  // <-   CPU_A6
#define PCFPIN_A13      22  // <-   CPU_A13
#define PCFPIN_V6       23  //  ->  MEM_A6 (contended memory)
#define PCFPIN_TS1      24  // <-   HAL_UA15
#define PCFPIN_TS2      25  // <-   HAL_UA14
#define PCFPIN_M7       26  //  ->  MEM_A7 (uncontended memory)
#define PCFPIN_M6       27  //  ->  MEM_A6 (uncontended memory)
#define PCFPIN_M5       28  //  ->  MEM_A5 (uncontended memory)
#define PCFPIN_M4       29  //  ->  MEM_A4 (uncontended memory)
#define PCFPIN_M3       30  //  ->  MEM_A3 (uncontended memory)
#define PCFPIN_M2       31  //  ->  MEM_A2 (uncontended memory)
#define PCFPIN_M1       32  //  ->  MEM_A1 (uncontended memory)
#define PCFPIN_M0       33  //  ->  MEM_A0 (uncontended memory)
#define PCFPIN_CAS      34  //  ->  MEM_CAS
#define PCFPIN_MREQD    35  // <-   CPU_MREQ with delay
#define PCFPIN_MREQ     36  // <-   CPU_MREQ
#define PCFPIN_A15      37  // <-   HAL_ULA15
#define PCFPIN_RD       38  // <-   CPU_RD
#define PCFPIN_WR       39  // <-   CPU_WR
#define PCFPIN_VCC      40

#define PCF_TS1         pcfPin[PCFPIN_TS1 - 1]
#define PCF_TS2         pcfPin[PCFPIN_TS2 - 1]
#define PCF_CAS         pcfPin[PCFPIN_CAS - 1]
#define PCF_MREQ        pcfPin[PCFPIN_MREQ - 1]
#define PCF_A15         pcfPin[PCFPIN_A15 - 1]
#define PCF_RD          pcfPin[PCFPIN_RD - 1]
#define PCF_WR          pcfPin[PCFPIN_WR - 1]

extern short            pcfPin[40];

#endif
