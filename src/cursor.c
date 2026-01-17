#include "types.h"

#include "bus.h"

typedef struct
{
    int     row;
    int     bit;
}
MAP;

static MAP      joyMap[5] = {{1, 2}, {0, 4}, {1, 4}, {1, 3}, {1, 0}};

static BYTE     joyData[2] = {0xff, 0xff};

void Cursor_Input(int joy, int bit, int state)
{
    (void)joy;

    MAP     *map = &joyMap[bit];

    joyData[map->row] |= (1 << map->bit);
    joyData[map->row] &= ~(state << map->bit); // state: 1=up 0=down
}

void Cursor_Read()
{
    if (BUS_IORQ && BUS_RD && (!BUS_A0))
    {
        if (!BUS_A11)
        {
            busDataOut &= joyData[0];
            busState = LOW;
        }
        if (!BUS_A12)
        {
            busDataOut &= joyData[1];
            busState = LOW;
        }
    }
}
