#ifndef __PSG__
#define __PSG__

#define PSGPIN_C        1   //  ->  channel C out
#define PSGPIN_TEST1    2   //      GI test purposes only
#define PSGPIN_VCC      3
#define PSGPIN_B        4   //  ->  channel B out
#define PSGPIN_A        5   //  ->  channel A out
#define PSGPIN_GND      6
#define PSGPIN_IOA7     7   // <->
#define PSGPIN_IOA6     8   // <->
#define PSGPIN_IOA5     9   // <->
#define PSGPIN_IOA4     10  // <->
#define PSGPIN_IOA3     11  // <->
#define PSGPIN_IOA2     12  // <->
#define PSGPIN_IOA1     13  // <->
#define PSGPIN_IOA0     14  // <->
#define PSGPIN_CLOCK    15  // <-   ULA_C
#define PSGPIN_RESET    16  // <-
#define PSGPIN_A8       17  // <-   VCC (as pin 3)
#define PSGPIN_BDIR     18  // <-   bus direction (0 read, 1 write)
#define PSGPIN_BC2      19  // <-   VCC (as pin 3)
#define PSGPIN_BC1      20  // <-   cpu io
#define PSGPIN_DA7      21  // <->
#define PSGPIN_DA6      22  // <->
#define PSGPIN_DA5      23  // <->
#define PSGPIN_DA4      24  // <->
#define PSGPIN_DA3      25  // <->
#define PSGPIN_DA2      26  // <->
#define PSGPIN_DA1      27  // <->
#define PSGPIN_DA0      28  // <->

#define PSG_C           psgPin[PSGPIN_C - 1]
#define PSG_B           psgPin[PSGPIN_B - 1]
#define PSG_A           psgPin[PSGPIN_A - 1]
#define PSG_CLOCK       psgPin[PSGPIN_CLOCK - 1]
#define PSG_RESET       psgPin[PSGPIN_RESET - 1]
#define PSG_BDIR        psgPin[PSGPIN_BDIR - 1]
#define PSG_BC1         psgPin[PSGPIN_BC1 - 1]

extern short            psgPin[28];

#endif
