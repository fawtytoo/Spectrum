#ifndef __BUS__
#define __BUS__

#define BUSPIN_A15          1       //  ->
#define BUSPIN_A13          2       //  ->
#define BUSPIN_D7           3       // <->
#define BUSPIN_NC1          4
#define BUSPIN_SLOT1        5
#define BUSPIN_D0           6       // <->
#define BUSPIN_D1           7       // <->
#define BUSPIN_D2           8       // <->
#define BUSPIN_D6           9       // <->
#define BUSPIN_D5           10      // <->
#define BUSPIN_D3           11      // <->
#define BUSPIN_D4           12      // <->
#define BUSPIN_INT          13      // <-
#define BUSPIN_NMI          14      // <-
#define BUSPIN_HALT         15      //  ->
#define BUSPIN_MREQ         16      //  ->
#define BUSPIN_IORQ         17      //  ->
#define BUSPIN_RD           18      //  ->
#define BUSPIN_WR           19      //  ->
#define BUSPIN_minus5V      20
#define BUSPIN_WAIT         21      // <-
#define BUSPIN_plus12V      22
#define BUSPIN_12VAC        23
#define BUSPIN_M1           24      //  ->
#define BUSPIN_RFSH         25      //  ->
#define BUSPIN_A8           26      //  ->
#define BUSPIN_A10          27      //  ->
#define BUSPIN_NC2          28
#define BUSPIN_A14          28 + 1  //  ->
#define BUSPIN_A12          28 + 2  //  ->
#define BUSPIN_plus5V       28 + 3
#define BUSPIN_plus9V       28 + 4
#define BUSPIN_SLOT2        28 + 5
#define BUSPIN_GND1         28 + 6
#define BUSPIN_GND2         28 + 7
#define BUSPIN_CK           28 + 8  //  ->
#define BUSPIN_A0           28 + 9  //  ->
#define BUSPIN_A1           28 + 10 //  ->
#define BUSPIN_A2           28 + 11 //  ->
#define BUSPIN_A3           28 + 12 //  ->
#define BUSPIN_NC3          28 + 13 //  ->  BUS_ULAIORQ on +2
#define BUSPIN_GND3         28 + 14
#define BUSPIN_NC4          28 + 15
#define BUSPIN_NC5          28 + 16
#define BUSPIN_NC6          28 + 17
#define BUSPIN_NC7          28 + 18
#define BUSPIN_BUSRQ        28 + 19 // <-
#define BUSPIN_RESET        28 + 20
#define BUSPIN_A7           28 + 21 //  ->
#define BUSPIN_A6           28 + 22 //  ->
#define BUSPIN_A5           28 + 23 //  ->
#define BUSPIN_A4           28 + 24 //  ->
#define BUSPIN_ROMCS        28 + 25 // <-
#define BUSPIN_BUSACK       28 + 26 //  ->
#define BUSPIN_A9           29 + 27 //  ->
#define BUSPIN_A11          28 + 28 //  ->

#define BUS_IORQ            busPin[BUSPIN_IORQ - 1]
#define BUS_RD              busPin[BUSPIN_RD - 1]
#define BUS_M1              busPin[BUSPIN_M1 - 1]
#define BUS_A12             busPin[BUSPIN_A12 - 1]
#define BUS_A0              busPin[BUSPIN_A0 - 1]
#define BUS_A7              busPin[BUSPIN_A7 - 1]
#define BUS_A6              busPin[BUSPIN_A6 - 1]
#define BUS_A5              busPin[BUSPIN_A5 - 1]
#define BUS_A11             busPin[BUSPIN_A11 - 1]

extern short                busPin[56];
extern BYTE                 busDataOut;
extern int                  busState;

#endif
