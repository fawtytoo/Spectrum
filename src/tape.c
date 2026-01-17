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
void MEM_Direct(WORD, BYTE *, int);

int TAPE_FastLoad(BYTE type, WORD address, WORD length)
{
    int     match = 0;

    if (TAPE_Ended())
    {
        return 0;
    }

    if (*curBlock->data == type && curBlock->length - 2 == length)
    {
        for (curBlock->pos = curBlock->data + 1; length != 0; length--, address++, curBlock->pos++)
        {
            MEM_Direct(address, curBlock->pos, 1);
        }

        match = 1;
    }

    curBlock = curBlock->next;

    return match;
}

static void SaveBlock(BLOCK *block, BYTE type, WORD address, WORD length)
{
    int     count;

    *block->pos++ = type;
    block->id = type & 128 ? 1 : 0;

    for (block->checksum = type; length != 0; length--, address++, block->pos++)
    {
        MEM_Direct(address, block->pos, 0);
        block->checksum ^= *block->pos;
    }

    *block->pos = block->checksum;
    block->checksum ^= block->checksum;

    GetTapeText(block);

    count = BLOCKCOUNT;
    if ((count % 10) == 9)
    {
        count++;
    }
    OUTPUT(block, Digit(count, 1));
}

void TAPE_FastSave(WORD header, WORD data)
{
    BLOCK       *block[2];
    WORD        length;
    BYTE        byte[2];

    if (tapeReadOnly == TRUE)
    {
        SYS_Print("\nUnable to save data");
        SYS_Print(" Tape either read only or corrupt");
        return;
    }

    SYS_Print("\nSaving data to tape ...");

    block[0] = NewBlock(17 + 2);
    SaveBlock(block[0], 0x00, header, 17);

    MEM_Direct(header + 11, &byte[0], 0);
    MEM_Direct(header + 12, &byte[1], 0);
    length = LE16(byte);

    block[1] = NewBlock(length + 2);
    SaveBlock(block[1], 0xff, data, length);

    FILE_Write(block[0]->data, block[0]->length);
    FILE_Write(block[1]->data, block[1]->length);
}
