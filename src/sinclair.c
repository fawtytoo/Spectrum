#include "types.h"

#include "bus.h"

static int      joyMap[2][5] = {{3, 4, 2, 1, 0}, {1, 0, 2, 3, 4}};

static BYTE     joyData[2] = {0xff, 0xff};

void Sinclair_Input(int joy, int bit, int state)
{
    bit = joyMap[joy][bit];

    joyData[joy] |= (1 << bit);
    joyData[joy] &= ~(state << bit); // state: 1=up 0=down
}

void Sinclair_Read()
{
    if (BUS_IORQ && BUS_RD && (!BUS_A0))
    {
        if (!BUS_A12)
        {
            busDataOut &= joyData[0];
            busState = LOW;
        }
        if (!BUS_A11)
        {
            busDataOut &= joyData[1];
            busState = LOW;
        }
    }
}
