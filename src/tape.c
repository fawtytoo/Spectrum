#include "spectrum.h"

// pulse -----------------------------------------------------------------------
#define CURPULSE(p)     curPulse = &p; pulseDo.lambda = p.lambda; pulseDo.repeat = p.repeat

typedef struct
{
    int     lambda;
    int     repeat;
}
PULSE;

static PULSE            pulsePilot[2] = {{2168, 8064}, {2168, 3224}};
static PULSE            pulseSync1 = {667, 1}, pulseSync2 = {735, 1};
static PULSE            pulseBit[2] = {{855, 2}, {1710, 2}};
static PULSE            pulseSilence[2] = {{SECOND, 1}, {SECOND * 2, 1}};
static PULSE            pulseDo = {1, 1};
static PULSE            *curPulse = &pulseSilence[0];

static int              pulseStage = 0;
static short            pulseWave = 0;

static BYTE             dataByte;
static int              dataCount;

// blocks ----------------------------------------------------------------------
#define LE16(d)         (d[0] | (d[1] << 8))

#define BLOCKCOUNT      tapeBlock.prev->index

#define OUTPUT(b, c)    SYS_Print(" %*i | %.10s | %s | %5i | %s", c, b->index, b->name, b->type, b->length - 2, b->checksum == 0 ? " ok " : "FAIL");

typedef struct _BLOCK   BLOCK;
struct _BLOCK
{
    BLOCK       *prev, *next;
    int         index;
    int         length;
    BYTE        *data, *end;
    BYTE        *pos;
    int         id;
    char        *type;
    char        name[10];
    BYTE        checksum;
};

static BLOCK            tapeBlock = {.prev = &tapeBlock, .next = &tapeBlock, .index = 0};
static BLOCK            *curBlock = &tapeBlock;

static char             *blockType[5] =
{
    "Program        ",
    "Number Array   ",
    "Character Array",
    "Code           ",
    " - - - - - - - "
};

// tape ------------------------------------------------------------------------
static int      tapeLoaded = FALSE;
static int      tapePlaying = FALSE;
static int      tapeFastSpeed = FALSE;
static int      tapeReadOnly = FALSE;

// pulse -----------------------------------------------------------------------
static void CheckPulse()
{
    if (pulseStage == 0)
    {
        CURPULSE(pulsePilot[curBlock->id]);
        pulseWave = 0;
        pulseStage = 1;

        curBlock->pos = curBlock->data;
    }
    else if (pulseStage == 1)
    {
        CURPULSE(pulseSync1);
        pulseStage = 2;
    }
    else if (pulseStage == 2)
    {
        CURPULSE(pulseSync2);
        pulseStage = 3;

        dataCount = 0;
    }
    else if (dataCount < 8 || curBlock->pos < curBlock->end)
    {
        dataCount &= 7;
        if (dataCount == 0)
        {
            dataByte = *curBlock->pos++;
        }

        CURPULSE(pulseBit[dataByte >> 7]);
        dataByte <<= 1;
        dataCount++;
    }
    else
    {
        curBlock = curBlock->next;

        if (curBlock->index == 0)
        {
            tapePlaying = FALSE;
        }
        else
        {
            CURPULSE(pulseSilence[curBlock->prev->id]);
            pulseStage = 0;
        }
    }
}

short TAPE_Input(short out)
{
    if (tapeLoaded == FALSE || tapePlaying == FALSE || tapeFastSpeed == TRUE)
    {
        return out;
    }

    if (pulseDo.lambda-- == 0)
    {
        pulseWave ^= 1;
        pulseDo.lambda = curPulse->lambda;
        if (--pulseDo.repeat == 0)
        {
            CheckPulse();
        }
    }

    return pulseWave;
}

// blocks ----------------------------------------------------------------------
static BLOCK *NewBlock(int length)
{
    BLOCK       *block = (BLOCK *)SYS_Malloc(sizeof(BLOCK));

    block->index = BLOCKCOUNT + 1;
    block->length = length;
    block->data = SYS_Malloc(length);
    block->end = block->data + length;
    block->pos = block->data;

    block->prev = tapeBlock.prev;
    block->next = &tapeBlock;
    tapeBlock.prev->next = block;
    tapeBlock.prev = block;

    return block;
}

static void FreeBlocks()
{
    BLOCK       *block = tapeBlock.next;

    while (block->index > 0)
    {
        SYS_Free(block->data);
        block = block->next;
        SYS_Free(block->prev);
    }

    tapeBlock.next = &tapeBlock;
    tapeBlock.prev = &tapeBlock;
}

// tape ------------------------------------------------------------------------
static void GetTapeText(BLOCK *block)
{
    int     i;
    char    *c;

    if (block->id == 0)
    {
        block->type = blockType[*(block->data + 1) & 3];
        c = (char *)(block->data + 2);
        for (i = 0; i < 10; i++, c++)
        {
            if (*c < ' ' || *c > 127)
            {
                block->name[i] = '.';
            }
            else
            {
                block->name[i] = *c;
            }
        }
    }
    else
    {
        block->type = blockType[4];
        for (i = 0; i < 10; i++)
        {
            block->name[i] = ' ';
        }
    }
}

static int TapeLoad(BYTE *data, int size)
{
    BLOCK       *block;
    int         length;

    while (size > 0)
    {
        if (size < 2)
        {
            return 1;
        }
        size -= 2;
        length = LE16(data);
        if (size < length || length < 2)
        {
            return 1;
        }

        data += sizeof(WORD);

        size -= length;

        block = NewBlock(length);
        block->checksum = 0x00;
        while (length-- > 0)
        {
            block->checksum ^= *data;
            *block->pos++ = *data++;
        }

        block->id = *block->data == 0x00 ? 0 : 1; // header/data
        GetTapeText(block);
    }

    return 0;
}

static int Digit(int number, int count)
{
    if (number > 9)
    {
        return Digit(number / 10, count + 1);
    }

    return count;
}

void TAPE_Load(BYTE *data, int size, int readonly)
{
    BLOCK       *block;
    int         count, c;
    int         error;

    tapeReadOnly = readonly;

    tapeLoaded = FALSE;

    error = TapeLoad(data, size);

    curBlock = &tapeBlock;

    count = BLOCKCOUNT;
    if (count > 0)
    {
        c = Digit(count, 1);

        block = tapeBlock.next;
        while (block->index > 0)
        {
            OUTPUT(block, c);

            block = block->next;
        }
        TAPE_Rewind(0);
    }
    else if (error == 0)
    {
        SYS_Print(" Empty tape!");
    }
    else
    {
        SYS_Print(" Invalid tape data !");
        return;
    }

    if (error == 1)
    {
        // this could be assumed to be data that could be over written
        // thoughts?
        tapeReadOnly = TRUE; // prevent saving
        SYS_Print(" Unknown trailing data");
    }

    tapeLoaded = TRUE;
}

int TAPE_BlockCount()
{
    return BLOCKCOUNT;
}

int TAPE_Rewind(int index)
{
    if (curBlock->index == 0 || index == 0)
    {
        curBlock = tapeBlock.next;
    }
    else if (curBlock->pos == curBlock->data && curBlock->prev->index > 0)
    {
        curBlock = curBlock->prev;
    }

    pulseStage = 0;
    CheckPulse();

    if (curBlock->index == 1 && index == 0)
    {
        tapePlaying = FALSE;
    }

    return curBlock->index;
}

int TAPE_FastForward()
{
    if (curBlock->index == 0)
    {
        curBlock = tapeBlock.next;
    }
    else if (curBlock->next->index > 0)
    {
        curBlock = curBlock->next;
    }
    else
    {
        // last block so no point in f/f
        return curBlock->index;
    }

    pulseStage = 0;
    CheckPulse();

    return curBlock->index;
}

void TAPE_Eject()
{
    FreeBlocks();

    tapePlaying = FALSE;
    tapeLoaded = FALSE;
    curBlock = &tapeBlock;
}

int TAPE_Ended()
{
    return curBlock->index == 0 ? TRUE : FALSE;
}

int TAPE_Playing()
{
    if (TAPE_Ended() == FALSE && tapeFastSpeed == FALSE)
    {
        tapePlaying ^= 1;
    }

    return tapePlaying;
}

int TAPE_Loaded()
{
    return tapeLoaded;
}

void TAPE_SpeedToggle()
{
    if (!tapePlaying)
    {
        tapeFastSpeed ^= 1;
    }
}

int TAPE_FastSpeed()
{
    return tapeLoaded == TRUE && tapeFastSpeed == TRUE;
}

// fast loading/saving ---------------------------------------------------------
#include "bus.h"

#define ROM_48K         0
#define ROM_128K        1

#define TAPE_DATA       0x0000

static BYTE     tapeRom[0x4000] =
{
    [0 ... 0x04bf] = 0,
    0x00, 0x00, 0xcd, 0xc6, 0x04, 0xc9, 0x4f, 0x7b, 0x32, 0x00, 0x00, 0x7a, 0x32, 0x00, 0x00, 0x79,
    0x32, 0x00, 0x00, 0xdd, 0x7e, 0x00, 0x32, 0x00, 0x00, 0xa9, 0x4f, 0xdd, 0x23, 0x1b, 0x7a, 0xb3,
    0x20, 0xf1, 0x79, 0x32, 0x00, 0x00, 0x37, 0xc9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    [0x04f0 ... 0x054f] = 0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x08, 0x15, 0xf3, 0x00, 0x00, 0x00, 0x00, 0x21, 0x3f,
    0x05, 0xe5, 0xdb, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xc0, 0xcd, 0x70, 0x05, 0xc9,
    0x08, 0xd0, 0x4f, 0x3a, 0x00, 0x00, 0xa9, 0xc0, 0x3a, 0x00, 0x00, 0xdd, 0x77, 0x00, 0xa9, 0x4f,
    0xdd, 0x23, 0x1b, 0x7a, 0xb3, 0x20, 0xf1, 0x3a, 0x00, 0x00, 0xa9, 0xc0, 0x37, 0xc9
};

static int      romType = ROM_48K;

void TAPE_Cycle()
{
    static BYTE     rom = 0x10; // assume 48k rom
    static int      ff1 = 0, ff2 = 0, rcvd = 0;
    static BYTE     size[2];

    if (romType == ROM_128K && BUS_IORQ && BUS_WR && (!BUS_A15) && (!BUS_A1))
    {
        // we need to monitor if the 128k roms swap
        //  as we should only override the subroutines in the 48k rom
        rom = busDataIn & 0x10;
    }

    BUS_ROMCS &= (!(BUS_RESET | ff2));
    ff1 &= BUS_ROMCS;
    ff2 &= BUS_ROMCS;

    if ((!ff1) && rom == 0x10 && tapeFastSpeed && BUS_M1)
    {
        if ((busAddress == 0x0556 || busAddress == 0x056c) && BUS_MREQ && BUS_RD)
        {
            curBlock->pos = curBlock->data;
        }
        else if (busAddress == 0x04c2 && BUS_MREQ && BUS_RD && tapeReadOnly == FALSE)
        {
            rcvd = 0;
        }
        else
        {
            return;
        }

        ff1 = 1;
        BUS_ROMCS = 1;
    }

    if (ff1 && BUS_MREQ)
    {
        if (BUS_RD)
        {
            if (busAddress == TAPE_DATA && curBlock->index > 0 && curBlock->pos < curBlock->end)
            {
                tapeRom[TAPE_DATA] = *curBlock->pos++;
            }
            busDataOut = tapeRom[busAddress];
            busState = LOW;

            if (busAddress == 0x04c5)
            {
                FILE_Write(curBlock->data, curBlock->length);
            }
            else if (busAddress == 0x056f)
            {
                if (curBlock->index > 0)
                {
                    curBlock = curBlock->next;
                }
            }
            else
            {
                return;
            }

            ff1 = 0;
            ff2 = 1;
        }
        else if (busAddress == TAPE_DATA && BUS_WR)
        {
            if (rcvd < 2)
            {
                size[rcvd] = busDataIn;
                rcvd++;
                if (rcvd == 2)
                {
                    curBlock = NewBlock(LE16(size) + 2);
                    SYS_Print("\nSaving data (size: %i)", LE16(size));
                }
            }
            else if (curBlock->index > 0 && curBlock->pos < curBlock->end)
            {
                *curBlock->pos++ = busDataIn;
            }
        }
    }
}

void TAPE_RomLock(int size)
{
    if (size == 32768)
    {
        romType = ROM_128K;
    }
}
