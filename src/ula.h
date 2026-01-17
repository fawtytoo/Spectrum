#ifndef __ULA__
#define __ULA__

#define ULAPIN_17M          1
#define ULAPIN_CAS          2   //  ->  MEM_CAS
#define ULAPIN_C            3   //  ->  PSG_CLOCK
#define ULAPIN_DMA0         4   //  ->  MEM_A0
#define ULAPIN_DMA1         5   //  ->  MEM_A1
#define ULAPIN_DMA2         6   //  ->  MEM_A2
#define ULAPIN_DMA3         7   //  ->  MEM_A3
#define ULAPIN_DMA4         8   //  ->  MEM_A4
#define ULAPIN_DMA5         9   //  ->  MEM_A5
#define ULAPIN_DMA6         10  //  ->  MEM_A6
#define ULAPIN_DMA7         11  //  ->  MEM_A7
#define ULAPIN_VB           12  // <-   MMU_Q3
#define ULAPIN_VCC2         13
#define ULAPIN_VCC1         14
#define ULAPIN_RD           15  // <-   CPU_RD
#define ULAPIN_WR           16  // <-   CPU_WR
#define ULAPIN_INT          17  //  ->  CPU_INT
#define ULAPIN_DRAMWE       18  //  ->  MEM_WR
#define ULAPIN_B            19  //  ->  blue
#define ULAPIN_G            20  //  ->  green
#define ULAPIN_R            21  //  ->  red
#define ULAPIN_BRIGHT       22  //  ->  brightness
#define ULAPIN_SYNC         23  //  ->  SYNC_VERT | SYNC_HORZ
#define ULAPIN_D0           24  // <->
#define ULAPIN_KB4          25  // <-   keyboard
#define ULAPIN_KB3          26  // <-
#define ULAPIN_D1           27  // <->
#define ULAPIN_D2           28  // <->
#define ULAPIN_KB2          29  // <-
#define ULAPIN_KB1          30  // <-
#define ULAPIN_D3           31  // <->
#define ULAPIN_KB0          32  // <-
#define ULAPIN_D4           33  // <->
#define ULAPIN_EAR          34  // <-   tape input
#define ULAPIN_MIC          35  //  ->  tape and speaker output
#define ULAPIN_D5           36  // <->
#define ULAPIN_D6           37  // <->
#define ULAPIN_D7           38  // <->
#define ULAPIN_PHICPU       39  //  ->  CPU_CLK
#define ULAPIN_IORQ         40  // <-   CPU_IOREQ
#define ULAPIN_ROMS         41  //  ->  ROM_CE (rom chip enable)
#define ULAPIN_RAS          42  //  ->  MEM_RAS and PCF_RAS
#define ULAPIN_A14          43  // <-   HAL_ULA14
#define ULAPIN_A15          44  // <-   HAL_ULA15
#define ULAPIN_MREQ         45  // <-   CPU_MREQ
#define ULAPIN__88M         46
#define ULAPIN_88M          47
#define ULAPIN_GND          48

#define ULA_CAS             ulaPin[ULAPIN_CAS - 1]
#define ULA_C               ulaPin[ULAPIN_C - 1]
#define ULA_DMA7            ulaPin[ULAPIN_DMA7 - 1]
#define ULA_VB              ulaPin[ULAPIN_VB - 1]
#define ULA_RD              ulaPin[ULAPIN_RD - 1]
#define ULA_WR              ulaPin[ULAPIN_WR - 1]
#define ULA_INT             ulaPin[ULAPIN_INT - 1]
#define ULA_DRAMWE          ulaPin[ULAPIN_DRAMWE - 1]
#define ULA_B               ulaPin[ULAPIN_B - 1]
#define ULA_G               ulaPin[ULAPIN_G - 1]
#define ULA_R               ulaPin[ULAPIN_R - 1]
#define ULA_BRIGHT          ulaPin[ULAPIN_BRIGHT - 1]
#define ULA_SYNC            ulaPin[ULAPIN_SYNC - 1]
#define ULA_EAR             ulaPin[ULAPIN_EAR - 1]
#define ULA_MIC             ulaPin[ULAPIN_MIC - 1]
#define ULA_PHICPU          ulaPin[ULAPIN_PHICPU - 1]
#define ULA_IORQ            ulaPin[ULAPIN_IORQ - 1]
#define ULA_ROMS            ulaPin[ULAPIN_ROMS - 1]
#define ULA_RAS             ulaPin[ULAPIN_RAS - 1]
#define ULA_A14             ulaPin[ULAPIN_A14 - 1]
#define ULA_A15             ulaPin[ULAPIN_A15 - 1]
#define ULA_MREQ            ulaPin[ULAPIN_MREQ - 1]

#define SYNC_HORZ           1
#define SYNC_VERT           2

extern short                ulaPin[48];

#endif
