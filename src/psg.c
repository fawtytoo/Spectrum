#include "types.h"

#include "psg.h"

// registers -------------------------------------------------------------------
#define REG_A_L             0
#define REG_A_H             1
#define REG_B_L             2
#define REG_B_H             3
#define REG_C_L             4
#define REG_C_H             5
#define REG_NOISE           6
#define REG_MIXER           7
#define REG_A_VOL           8
#define REG_B_VOL           9
#define REG_C_VOL           10
#define REG_ENV_L           11
#define REG_ENV_H           12
#define REG_ENV_SHAPE       13
#define REG_IOA             14 // unused
#define REG_IOB             15 // unused on ay-3-8912

static int                  psgRegister = 0;
static BYTE                 psgData[16];

// envelopes -------------------------------------------------------------------
#define ENV_DOWN    15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
#define ENV_UP       0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
#define ENV_HIGH    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
#define ENV_LOW      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0

static const int    psgEnvelope[16][48] =
{
    {ENV_DOWN,  ENV_LOW,  ENV_LOW},
    {ENV_DOWN,  ENV_LOW,  ENV_LOW},
    {ENV_DOWN,  ENV_LOW,  ENV_LOW},
    {ENV_DOWN,  ENV_LOW,  ENV_LOW},
    {  ENV_UP,  ENV_LOW,  ENV_LOW},
    {  ENV_UP,  ENV_LOW,  ENV_LOW},
    {  ENV_UP,  ENV_LOW,  ENV_LOW},
    {  ENV_UP,  ENV_LOW,  ENV_LOW},
    {ENV_DOWN, ENV_DOWN, ENV_DOWN},
    {ENV_DOWN,  ENV_LOW,  ENV_LOW},
    {ENV_DOWN,   ENV_UP, ENV_DOWN},
    {ENV_DOWN, ENV_HIGH, ENV_HIGH},
    {  ENV_UP,   ENV_UP,   ENV_UP},
    {  ENV_UP, ENV_HIGH, ENV_HIGH},
    {  ENV_UP, ENV_DOWN,   ENV_UP},
    {  ENV_UP,  ENV_LOW,  ENV_LOW}
};

static int          psgEnvLambda = 0;
static int          psgEnvPos = 0;
static int          psgEnvVolume = 0;

// noise -----------------------------------------------------------------------
static int      psgNoiseLambda = 0;
static int      psgNoiseSeed = 1;
static int      psgNoisePhase = 0;

// channels --------------------------------------------------------------------
#define CHAN_A      0
#define CHAN_B      1
#define CHAN_C      2

typedef struct
{
    int     phase;
    int     lambda;
    BYTE    *l, *h;
    int     tone, noise;
    int     volume;
    int     envVolOn;
    int     *volOut[2];
}
PSGCHAN;

static PSGCHAN      psgChan[3] =
{
    {.l = &psgData[REG_A_L], .h = &psgData[REG_A_H], .volOut = {&psgChan[CHAN_A].volume, &psgEnvVolume}},
    {.l = &psgData[REG_B_L], .h = &psgData[REG_B_H], .volOut = {&psgChan[CHAN_B].volume, &psgEnvVolume}},
    {.l = &psgData[REG_C_L], .h = &psgData[REG_C_H], .volOut = {&psgChan[CHAN_C].volume, &psgEnvVolume}}
};

// DAC -------------------------------------------------------------------------
// 8192.0f / pow(sqrt(2), (15 - volume))
static const int    psgDigitalToAnalog[16] = {45, 63, 90, 127, 181, 255, 362, 511, 724, 1023, 1448, 2047, 2896, 4095, 5792, 8192};

// external --------------------------------------------------------------------
short       psgPin[28];
BYTE        psgDataOut;
int         psgState;

 // registers ------------------------------------------------------------------
static void PSG_Register(BYTE data)
{
    psgRegister = data & 0xf;
}

static void PSG_SetData(BYTE data)
{
    switch (psgRegister)
    {
      case REG_A_L:
      case REG_B_L:
      case REG_C_L:
        break;

      case REG_A_H:
      case REG_B_H:
      case REG_C_H:
        data &= 15;
        break;

      case REG_NOISE:
        data &= 31;
        break;

      case REG_MIXER:
        psgChan[CHAN_A].tone = data & 1;
        psgChan[CHAN_B].tone = (data & 2) >> 1;
        psgChan[CHAN_C].tone = (data & 4) >> 2;
        psgChan[CHAN_A].noise = (data & 8) >> 3;
        psgChan[CHAN_B].noise = (data & 16) >> 4;
        psgChan[CHAN_C].noise = (data & 32) >> 5;
        break;

      case REG_A_VOL:
        data &= 31;
        psgChan[CHAN_A].volume = data & 15;
        psgChan[CHAN_A].envVolOn = data >> 4;
        break;

      case REG_B_VOL:
        data &= 31;
        psgChan[CHAN_B].volume = data & 15;
        psgChan[CHAN_B].envVolOn = data >> 4;
        break;

      case REG_C_VOL:
        data &= 31;
        psgChan[CHAN_C].volume = data & 15;
        psgChan[CHAN_C].envVolOn = data >> 4;
        break;

      case REG_ENV_L:
      case REG_ENV_H:
        break;

      case REG_ENV_SHAPE:
        data &= 15;
        psgEnvLambda = 0;
        psgEnvPos = 0;
        break;
    }

    psgData[psgRegister] = data;
}

// generate --------------------------------------------------------------------
static int Lambda_Frequency(int lambda)
{
    if (lambda == 0)
    {
        lambda = 1;
    }

    return lambda * 16;
}

static int PSG_Generate(PSGCHAN *chan)
{
    if (chan->lambda == 0)
    {
        chan->lambda = Lambda_Frequency(*chan->h * 256 + *chan->l);
        chan->phase ^= 1;
    }
    chan->lambda--;

    return *chan->volOut[chan->envVolOn] * ((chan->phase | chan->tone) & (psgNoisePhase | chan->noise));
}

// cycle -----------------------------------------------------------------------
void PSG_Cycle()
{
    int     bit;

    if (psgNoiseLambda == 0)
    {
        psgNoiseLambda = Lambda_Frequency(psgData[REG_NOISE]);
        bit = (psgNoiseSeed ^ (psgNoiseSeed >> 3)) & 1;
        psgNoiseSeed = (psgNoiseSeed >> 1) | (bit << 16);
        psgNoisePhase ^= bit;
    }
    psgNoiseLambda--;

    if (psgEnvLambda == 0)
    {
        psgEnvLambda = Lambda_Frequency(psgData[REG_ENV_H] * 256 + psgData[REG_ENV_L]);
        psgEnvVolume = psgEnvelope[psgData[REG_ENV_SHAPE]][psgEnvPos];
        psgEnvPos++;
        if (psgEnvPos == 48)
        {
            psgEnvPos = 16;
        }
    }
    psgEnvLambda--;

    PSG_A = psgDigitalToAnalog[PSG_Generate(&psgChan[CHAN_A])];
    PSG_B = psgDigitalToAnalog[PSG_Generate(&psgChan[CHAN_B])];
    PSG_C = psgDigitalToAnalog[PSG_Generate(&psgChan[CHAN_C])];
}

void PSG_Read()
{
    psgDataOut = 0xff;
    psgState = HIGH;

    if (PSG_BC1 && (!PSG_BDIR))
    {
        psgDataOut = psgData[psgRegister];
        psgState = LOW;
    }
}

void PSG_Write(BYTE data)
{
    if (PSG_BDIR)
    {
        if (PSG_BC1)
        {
            PSG_Register(data);
        }
        else
        {
            PSG_SetData(data);
        }
    }
}

void PSG_Reset()
{
    PSG_Register(REG_A_L);
    PSG_SetData(0);
    PSG_Register(REG_A_H);
    PSG_SetData(0);
    PSG_Register(REG_B_L);
    PSG_SetData(0);
    PSG_Register(REG_B_H);
    PSG_SetData(0);
    PSG_Register(REG_C_L);
    PSG_SetData(0);
    PSG_Register(REG_C_H);
    PSG_SetData(0);
    PSG_Register(REG_NOISE);
    PSG_SetData(0);
    PSG_Register(REG_MIXER);
    PSG_SetData(0);
    PSG_Register(REG_A_VOL);
    PSG_SetData(0);
    PSG_Register(REG_B_VOL);
    PSG_SetData(0);
    PSG_Register(REG_C_VOL);
    PSG_SetData(0);
    PSG_Register(REG_ENV_L);
    PSG_SetData(0);
    PSG_Register(REG_ENV_H);
    PSG_SetData(0);
    PSG_Register(REG_ENV_SHAPE);
    PSG_SetData(0);
    //psgData[REG_IOA] = 0;
    //psgData[REG_IOB] = 0;
}
