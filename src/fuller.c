#include "types.h"

#include "bus.h"

static int      joyMap[5] = {3, 2, 1, 0, 7};

static BYTE     joyData = 0xff;

void Fuller_Input(int joy, int bit, int state)
{
    (void)joy;

    bit = joyMap[bit];

    joyData |= (1 << bit);
    joyData &= ~(state << bit); // state: 1=up 0=down
}

void Fuller_Read()
{
    if ((!BUS_M1) && BUS_IORQ && BUS_RD && (!BUS_A7))
    {
        busDataOut &= joyData;
        busState = LOW;
    }
}
