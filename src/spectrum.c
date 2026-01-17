#include "common.h"

#include "cpu.h"
#include "mem.h"
#include "ula.h"
#include "psg.h"
#include "bus.h"

#include "spectrum.h"

// timer -----------------------------------------------------------------------
typedef struct
{
    int     acc;
    int     rate;
    int     remainder;
    int     divisor;
}
TIMER;

static TIMER    timerAudio =
{
    .acc = 0, .rate = 0, .remainder = SAMPLERATE, .divisor = 6988800
};
static TIMER    timerTape =
{
    .acc = 0, .rate = 0, .remainder = SECOND, .divisor = 6988800
};

// video -----------------------------------------------------------------------
static const BYTE   tvColour[2] = {0x00, 0xff};
static const BYTE   tvBright[2] = {0xa0, 0xff};

static int          tvTopLine = 0;

// audio -----------------------------------------------------------------------
#define BUFFERSIZE      4095
#define BUFFER(b, c)    b.buffer[(b.c) & BUFFERSIZE]

#define LEFT            0
#define RIGHT           1

typedef struct
{
    short       buffer[BUFFERSIZE + 1];
    WORD        read;
    WORD        write;
}
BUFFER;

static BUFFER           audioOutput, audioInput;

static short            silence = 0; // constant
static short            *audioMap[2][3] = {{&PSG_A, &PSG_B, &PSG_C}, {&PSG_A, &PSG_B, &PSG_C}};

// keyboard --------------------------------------------------------------------
typedef struct
{
    int     row;
    int     bit;
}
MAP;

static const MAP    keyMap[40] =
{
    {4, 0}, {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {4, 4}, {4, 3}, {4, 2}, {4, 1},
    {1, 0}, {7, 4}, {0, 3}, {1, 2}, {2, 2}, {1, 3}, {1, 4}, {6, 4}, {5, 2}, {6, 3},
    {6, 2}, {6, 1}, {7, 2}, {7, 3}, {5, 1}, {5, 0}, {2, 0}, {2, 3}, {1, 1}, {2, 4},
    {5, 3}, {0, 4}, {2, 1}, {0, 2}, {5, 4}, {0, 1}, {0, 0}, {7, 1}, {7, 0}, {6, 0}
};

static BYTE         keyRow[8] = {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};

static BYTE         discard; // unwanted key data
static BYTE         *keyData[2] = {&ulaKeyData, &discard};

// joystick --------------------------------------------------------------------
static int      joyMap[2][5] =
{
    {KEY_P, KEY_O, KEY_A, KEY_Q, KEY_SP},
    {KEY_8, KEY_5, KEY_6, KEY_7, KEY_0}
};

void (*SPECTRUM_Joystick)(int, int, int) = Sinclair_Input;

// expansion bus ---------------------------------------------------------------
short       busPin[56];
BYTE        busDataOut;
int         busState;

// timer -----------------------------------------------------------------------
static void Timer_Set(TIMER *timer, int numerator, int divisor)
{
    timer->acc = 0;
    timer->rate = numerator / divisor;
    timer->remainder = numerator - timer->rate * divisor;
    timer->divisor = divisor;
}

static int Timer_Update(TIMER *timer)
{
    timer->acc += timer->remainder;
    if (timer->acc < timer->divisor)
    {
        return timer->rate;
    }

    timer->acc -= timer->divisor;

    return timer->rate + 1;
}

// spectrum --------------------------------------------------------------------
void SPECTRUM_TVScan(BYTE *screen)
{
    static short    tape = 0;

    int             column = 0;
    short           beep;

    screen += tvTopLine;

    do
    {
        ULA_VB = MMU_Q3;
        ULA_RD = CPU_RD;
        ULA_WR = CPU_WR;
        ULA_IORQ = CPU_IORQ & (!CPU_A0);
        ULA_A14 = HAL_ULA14;
        ULA_A15 = HAL_ULA15;
        ULA_MREQ = CPU_MREQ;
        ULA_Cycle();

        // contended memory
        memBankIndex = 1;
        MEM_WE = ULA_DRAMWE;
        MEM_RAS = ULA_RAS;
        MEM_CAS = ULA_CAS;
        if (ulaState == LOW) // select address bus
        {
            memAddress = ulaAddress;
        }
        else if (CPU_MREQ)
        {
            // both HAL_VA15 and HAL_VA14 go via IC30
            memAddress = (HAL_VA15 << 15) | (HAL_VA14 << 14) | pcfAddressOut[1];
            memDataIn = cpuData;
        }
        MEM_Cycle();
        if (ulaState == LOW && memState == LOW)
        {
            ulaDataIn = memDataOut;
        }

        CPU_CLK = ULA_PHICPU;
        CPU_INT = ULA_INT;
        if (CPU_CLK)
        {
            // io reading

            // expansion bus first ...
            BUS_IORQ = CPU_IORQ;
            BUS_RD = CPU_RD;
            BUS_M1 = CPU_M1;
            BUS_A12 = CPU_A12;
            BUS_A0 = CPU_A0;
            BUS_A7 = CPU_A7;
            BUS_A6 = CPU_A6;
            BUS_A5 = CPU_A5;
            BUS_A11 = CPU_A11;
            busDataOut = 0xff;
            busState = HIGH;
            Sinclair_Read();
            Cursor_Read();
            Kempston_Read();
            Fuller_Read();

            // ... then internal
            PSG_Read();
            ULA_Read();

            if ((psgState | ulaState | busState) == LOW)
            {
                cpuData = psgDataOut & ulaDataOut & busDataOut;
            }
            else if (CPU_IORQ) // no response
            {
                if (CPU_RD && memState == LOW)
                {
                    cpuData = memDataOut;
                }
                else if (CPU_M1)
                {
                    cpuData = busDataOut;
                }
            }

            // io writing
            PSG_Write(cpuData);
            ULA_Write(cpuData);

            // memory
            ROM_CE = ULA_ROMS;
            ROM_OE = CPU_MREQ;
            ROM_A14 = MMU_Q4;
            if (ROM_CE && ROM_OE)
            {
                if (TAPE_FastSpeed())
                {
                    ROM_LdBytes();  // loading
                    ROM_SaContrl(); // saving
                }

                romAddress = cpuAddress & 0x3fff;
                ROM_Cycle();
                cpuData = romData;
            }
            else if (CPU_MREQ)
            {
                if (PCF_CAS) // uncontended memory
                {
                    memBankIndex = 0;
                    MEM_WE = CPU_WR;
                    MEM_RAS = CPU_MREQ;
                    MEM_CAS = PCF_CAS;
                    memAddress = pcfAddressOut[0];
                    memDataIn = cpuData;
                    MEM_Cycle();
                }

                // read from either uncontended or contended memory
                if (memState == LOW)
                {
                    cpuData = memDataOut;
                }
            }

            CPU_Cycle();

            // decoding ...
            HAL_A15 = CPU_A15;
            HAL_A14 = CPU_A14;
            HAL_A1 = CPU_A1;
            HAL_IORQ = CPU_IORQ;
            HAL_B0 = MMU_Q0;
            HAL_B1 = MMU_Q1;
            HAL_B2 = MMU_Q2;
            HAL_RD = CPU_RD;
            HAL_WR = CPU_WR;
            HAL_RFSH = CPU_RFSH;
            HAL_Cycle();

            MMU_CP = HAL_BANK & (!MMU_Q5);
            if (MMU_CP || MMU_CLR)
            {
                MMU_D0 = cpuData & 1;
                MMU_D1 = (cpuData >> 1) & 1;
                MMU_D2 = (cpuData >> 2) & 1;
                MMU_D3 = (cpuData >> 3) & 1;
                MMU_D4 = (cpuData >> 4) & 1;
                MMU_D5 = (cpuData >> 5) & 1;
                MMU_Cycle();
                MMU_CLR = 0;
            }

            pcfAddressIn = cpuAddress & 0x3fff;
            PCF_TS1 = HAL_UA15;
            PCF_TS2 = HAL_UA14;
            PCF_MREQ = CPU_MREQ;
            PCF_A15 = HAL_ULA15;
            PCF_RD = CPU_RD;
            PCF_WR = CPU_WR;
            PCF_Cycle();

            PSG_BDIR = HAL_PSG & (!CPU_RD);
            PSG_BC1 = HAL_PSG & CPU_A14;

            ulaKeyData = 0b00011111;
            *keyData[CPU_A8] &= keyRow[0];
            *keyData[CPU_A9] &= keyRow[1];
            *keyData[CPU_A10] &= keyRow[2];
            *keyData[CPU_A11] &= keyRow[3];
            *keyData[CPU_A12] &= keyRow[4];
            *keyData[CPU_A13] &= keyRow[5];
            *keyData[CPU_A14] &= keyRow[6];
            *keyData[CPU_A15] &= keyRow[7];
        }

        PSG_CLOCK = ULA_C;
        if (PSG_CLOCK)
        {
            PSG_Cycle();
        }

        // video output
        *(screen + column++) = tvColour[ULA_R] & tvBright[ULA_BRIGHT];
        *(screen + column++) = tvColour[ULA_G] & tvBright[ULA_BRIGHT];
        *(screen + column++) = tvColour[ULA_B] & tvBright[ULA_BRIGHT];
        if (ULA_SYNC & SYNC_HORZ)
        {
            ULA_SYNC &= SYNC_VERT;
            screen += WIDTH * BPP;
            column = 0;
        }

        // tape input
        if (Timer_Update(&timerTape)) // maintain constant tape speed
        {
            tape = TAPE_Input(ULA_MIC >> 2); // speaker bit as input
        }

        ULA_EAR = BUFFER(audioInput, read) | tape;

        beep = ULA_MIC | (ULA_EAR << 1);

        // downsample audio to SAMPLERATE
        if (Timer_Update(&timerAudio) == 0)
        {
            continue;
        }

        if (audioInput.read != audioInput.write)
        {
            audioInput.read++;
        }

        BUFFER(audioOutput, write + LEFT) = beep << 10;
        BUFFER(audioOutput, write + RIGHT) = beep << 10;
        BUFFER(audioOutput, write + LEFT) += *audioMap[LEFT][0];
        BUFFER(audioOutput, write + LEFT) += *audioMap[LEFT][1];
        BUFFER(audioOutput, write + LEFT) += *audioMap[LEFT][2];
        BUFFER(audioOutput, write + RIGHT) += *audioMap[RIGHT][0];
        BUFFER(audioOutput, write + RIGHT) += *audioMap[RIGHT][1];
        BUFFER(audioOutput, write + RIGHT) += *audioMap[RIGHT][2];

        audioOutput.write += 2;
    }
    while (ULA_SYNC ^ SYNC_VERT);
}

void SPECTRUM_Reset()
{
    CPU_Reset();
    PSG_Reset();
    MMU_CLR = 1;
}

// audio -----------------------------------------------------------------------
void SPECTRUM_AudioOut(short out[2], int *sync)
{
    static int      samples = 0;

    if (samples == 0)
    {
        samples = SAMPLES;
        *sync = TRUE;
    }
    samples--;

    out[LEFT] = BUFFER(audioOutput, read + LEFT);
    out[RIGHT] = BUFFER(audioOutput, read + RIGHT);

    if (audioOutput.read != audioOutput.write)
    {
        audioOutput.read += 2;
    }
}

void SPECTRUM_AudioIn(short in)
{
    BUFFER(audioInput, write) = in;

    audioInput.write++;
}

void SPECTRUM_AudioSetup(int setup)
{
    if (setup == AY_ABC)
    {
        audioMap[LEFT][0] = &PSG_A;
        audioMap[LEFT][1] = &PSG_B;
        audioMap[LEFT][2] = &silence;
        audioMap[RIGHT][0] = &PSG_B;
        audioMap[RIGHT][1] = &PSG_C;
        audioMap[RIGHT][2] = &silence;
    }
    else if (setup == AY_ACB)
    {
        audioMap[LEFT][0] = &PSG_A;
        audioMap[LEFT][1] = &PSG_C;
        audioMap[LEFT][2] = &silence;
        audioMap[RIGHT][0] = &PSG_C;
        audioMap[RIGHT][1] = &PSG_B;
        audioMap[RIGHT][2] = &silence;
    }
    else if (setup == AY_MONO)
    {
        audioMap[LEFT][0] = &PSG_A;
        audioMap[LEFT][1] = &PSG_B;
        audioMap[LEFT][2] = &PSG_C;
        audioMap[RIGHT][0] = &PSG_A;
        audioMap[RIGHT][1] = &PSG_B;
        audioMap[RIGHT][2] = &PSG_C;
    }
}

// video -----------------------------------------------------------------------
void SPECTRUM_UlaType(int type)
{
    if (type == ULA_48K || type == ULA_128K)
    {
        Timer_Set(&timerAudio, SAMPLERATE, ULA_Type(type) * FPS);
        Timer_Set(&timerTape, SECOND, timerAudio.divisor);
    }

    if (type == ULA_48K)
    {
        tvTopLine = 0;
    }
    else if (type == ULA_128K)
    {
        tvTopLine = WIDTH * BPP;
    }
}

// input -----------------------------------------------------------------------
void SPECTRUM_Keyboard(int key, int state)
{
    const MAP       *map = &keyMap[key];

    keyRow[map->row] |= (1 << map->bit);
    keyRow[map->row] &= ~(state << map->bit); // state: 1=up 0=down
}

void Joystick_Input(int joy, int bit, int state)
{
    SPECTRUM_Keyboard(joyMap[joy][bit], state);
}

int SPECTRUM_GetJoyKey(int joy, int bit)
{
    return joyMap[joy][bit];
}

void SPECTRUM_SetJoyKey(int joy, int bit, int key)
{
    joyMap[joy][bit] = key;
}

void SPECTRUM_JoystickSelect(int joy)
{
    if (joy == JOY_SINCLAIR)
    {
        SPECTRUM_Joystick = Sinclair_Input;
    }
    else if (joy == JOY_CURSOR)
    {
        SPECTRUM_Joystick = Cursor_Input;
    }
    else if (joy == JOY_KEMPSTON)
    {
        SPECTRUM_Joystick = Kempston_Input;
    }
    else if (joy == JOY_FULLER)
    {
        SPECTRUM_Joystick = Fuller_Input;
    }
    else if (joy == JOY_PROGRAM)
    {
        SPECTRUM_Joystick = Joystick_Input;
    }
}
