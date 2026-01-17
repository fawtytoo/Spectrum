#include "types.h"

#include "bus.h"

static BYTE     joyData = 0x00;

void Kempston_Input(int joy, int bit, int state)
{
    (void)joy;

    joyData &= ~(1 << bit);
    joyData |= (state << bit); // state: 0=up 1=down
}

void Kempston_Read()
{
    if ((!BUS_M1) && BUS_IORQ && BUS_RD && (!BUS_A7) && (!BUS_A6) && (!BUS_A5))
    {
        busDataOut &= joyData;
        busState = LOW;
    }
}
