//  Copyright 2026      by Steve Clark

//  This software is provided 'as-is', without any express or implied
//  warranty.  In no event will the authors be held liable for any damages
//  arising from the use of this software.

//  Permission is granted to anyone to use this software for any purpose,
//  including commercial applications, and to alter it and redistribute it
//  freely, subject to the following restrictions:

//  1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software
//     in a product, an acknowledgment in the product documentation would be
//     appreciated but is not required. 
//  2. Altered source versions must be plainly marked as such, and must not be
//     misrepresented as being the original software.
//  3. This notice may not be removed or altered from any source distribution.

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
