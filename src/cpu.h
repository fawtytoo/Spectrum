#ifndef __CPU__
#define __CPU__

#define CPUPIN_A11          1   //  ->
#define CPUPIN_A12          2   //  ->
#define CPUPIN_A13          3   //  ->
#define CPUPIN_A14          4   //  ->
#define CPUPIN_A15          5   //  ->
#define CPUPIN_CLK          6   // <-   ULA_PHICPU
#define CPUPIN_D4           7   // <->
#define CPUPIN_D3           8   // <->
#define CPUPIN_D5           9   // <->
#define CPUPIN_D6           10  // <->
#define CPUPIN_VCC          11
#define CPUPIN_D2           12  // <->
#define CPUPIN_D7           13  // <->
#define CPUPIN_D0           14  // <->
#define CPUPIN_D1           15  // <->
#define CPUPIN_INT          16  // <-   ULA_INT
#define CPUPIN_NMI          17  // <-
#define CPUPIN_HALT         18  //  ->
#define CPUPIN_MREQ         19  //  ->
#define CPUPIN_IORQ         20  //  ->
#define CPUPIN_RD           21  //  ->
#define CPUPIN_WR           22  //  ->
#define CPUPIN_BUSACK       23  //  ->
#define CPUPIN_WAIT         24  // <-
#define CPUPIN_BUSRQ        25  // <-
#define CPUPIN_RESET        26  // <-
#define CPUPIN_M1           27  //  ->  BUS_M1
#define CPUPIN_RFSH         28  //  ->  HAL_RFSH
#define CPUPIN_GND          29
#define CPUPIN_A0           30  //  ->
#define CPUPIN_A1           31  //  ->
#define CPUPIN_A2           32  //  ->
#define CPUPIN_A3           33  //  ->
#define CPUPIN_A4           34  //  ->
#define CPUPIN_A5           35  //  ->
#define CPUPIN_A6           36  //  ->
#define CPUPIN_A7           37  //  ->
#define CPUPIN_A8           38  //  ->
#define CPUPIN_A9           39  //  ->
#define CPUPIN_A10          40  //  ->

#define CPU_A11             cpuPin[CPUPIN_A11 - 1]
#define CPU_A12             cpuPin[CPUPIN_A12 - 1]
#define CPU_A13             cpuPin[CPUPIN_A13 - 1]
#define CPU_A14             cpuPin[CPUPIN_A14 - 1]
#define CPU_A15             cpuPin[CPUPIN_A15 - 1]
#define CPU_CLK             cpuPin[CPUPIN_CLK - 1]
#define CPU_INT             cpuPin[CPUPIN_INT - 1]
#define CPU_HALT            cpuPin[CPUPIN_HALT - 1]
#define CPU_MREQ            cpuPin[CPUPIN_MREQ - 1]
#define CPU_IORQ            cpuPin[CPUPIN_IORQ - 1]
#define CPU_RD              cpuPin[CPUPIN_RD - 1]
#define CPU_WR              cpuPin[CPUPIN_WR - 1]
#define CPU_M1              cpuPin[CPUPIN_M1 - 1]
#define CPU_RFSH            cpuPin[CPUPIN_RFSH - 1]
#define CPU_A0              cpuPin[CPUPIN_A0 - 1]
#define CPU_A1              cpuPin[CPUPIN_A1 - 1]
#define CPU_A2              cpuPin[CPUPIN_A2 - 1]
#define CPU_A3              cpuPin[CPUPIN_A3 - 1]
#define CPU_A4              cpuPin[CPUPIN_A4 - 1]
#define CPU_A5              cpuPin[CPUPIN_A5 - 1]
#define CPU_A6              cpuPin[CPUPIN_A6 - 1]
#define CPU_A7              cpuPin[CPUPIN_A7 - 1]
#define CPU_A8              cpuPin[CPUPIN_A8 - 1]
#define CPU_A9              cpuPin[CPUPIN_A9 - 1]
#define CPU_A10             cpuPin[CPUPIN_A10 - 1]

extern short                cpuPin[40];

#endif
