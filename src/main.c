#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <SDL2/SDL.h>

#include "spectrum.h"

#include "headsup.h"

#define VIEWWIDTH           352
#define VIEWHEIGHT          264

#define TITLE               "ZX Spectrum"

typedef void (*EVENT)(void);

// sdl -------------------------------------------------------------------------
#define FULLSCREEN          SDL_SetWindowFullscreen(sdlWindow, rectWindowZoom ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)

#define MOUSE_TIMEOUT       FPS * 1

static SDL_AudioDeviceID    sdlAudioOut, sdlAudioIn;
static SDL_Window           *sdlWindow;
static SDL_Renderer         *sdlRenderer;
static SDL_Texture          *texScreen, *texTarget;
static SDL_Surface          *sdlSurface;
static SDL_Rect             rectScreen[2] =
{
    {0, 28, VIEWWIDTH, VIEWHEIGHT}, {48, 64, 256, 192}
};
static SDL_Rect             rectTarget[2][2] =
{
    {
        {0, 0, VIEWWIDTH, VIEWHEIGHT}, {0, 0, 256, 192}
    },
    {
        {0, 0, VIEWWIDTH, VIEWHEIGHT}, {0, 0, 256, 192}
    }
};
static SDL_Rect             rectWindow[2] =
{
    {0, 0, VIEWWIDTH, VIEWHEIGHT}, {0, 0, 0, 0}
};
static int                  sdlLastSym = SDLK_UNKNOWN;

static int                  rectScreenZoom = 0;
static int                  rectWindowZoom = 0;

static int                  mouseHideTimer = MOUSE_TIMEOUT;

// video -----------------------------------------------------------------------
#define TEXT_ULA_48K        "48K video mode"
#define TEXT_ULA_128K       "128K video mode"

static int                  vSync = FALSE; // for video syncing
static int                  ulaType = ULA_48K;
static char                 *ulaText = TEXT_ULA_48K;

// audio -----------------------------------------------------------------------
#define TEXT_AY_ABC         "A            B            C"
#define TEXT_AY_ACB         "A            C            B"
#define TEXT_AY_MONO        "Mono"

static int                  audioSetup = AY_MONO;
static char                 *audioText = TEXT_AY_MONO;

static Uint16               audioPlay = 0xffff;
static short                audioInput = 1;

// power -----------------------------------------------------------------------
static int                  emuPower = TRUE;
static int                  emuPaused = FALSE;
static EVENT                Emulate;

// headsup ---------------------------------------------------------------------
#define HU_TIMEOUT_KEY      FPS * 2
#define HU_TIMEOUT          FPS * 4

#define HU_GFX_INVERT       (Uint8 [3]){0xa0, 0xff, 0x00}
#define HU_GFX_BLACK        (Uint8 [3]){0x00, 0x00, 0x00}
#define HU_GFX_WHITE        (Uint8 [3]){0x00, 0xff, 0xff}

typedef struct
{
    int         width;
    int         height;
    int         pos;
}
HEADSUP;

static HEADSUP              huJoystick[5] =
{
    {.width = 195, .height = 32, .pos = 30 * WIDTH + 78},
    {.width = 162, .height = 32, .pos = 30 * WIDTH + 95},
    {.width = 205, .height = 32, .pos = 30 * WIDTH + 73},
    {.width = 94,  .height = 32, .pos = 30 * WIDTH + 129},
    {.width = 246, .height = 32, .pos = 30 * WIDTH + 53}
};
static int                  huJoystickTimer = 0;

static HEADSUP              huProgram[11] =
{
    {.width = 8, .height = 16, .pos = 29 * WIDTH + 64},
    {.width = 8, .height = 16, .pos = 47 * WIDTH + 64},
    {.width = 8, .height = 16, .pos = 38 * WIDTH + 46},
    {.width = 8, .height = 16, .pos = 38 * WIDTH + 82},
    {.width = 8, .height = 16, .pos = 38 * WIDTH + 112},
    {.width = 8, .height = 16, .pos = 29 * WIDTH + 240},
    {.width = 8, .height = 16, .pos = 47 * WIDTH + 240},
    {.width = 8, .height = 16, .pos = 38 * WIDTH + 222},
    {.width = 8, .height = 16, .pos = 38 * WIDTH + 258},
    {.width = 8, .height = 16, .pos = 38 * WIDTH + 288},
    {.width = 8, .height = 16, .pos = 38 * WIDTH + 140}
};
static int                  huProgramTimer = 0;

static HEADSUP              huMessage =
{
    .width = 8, .height = 16, .pos = 266 * WIDTH
};
static char                 *huMessageText = TITLE;
static int                  huMessageTimer = 0;

static HEADSUP              huPaused =
{
    .width = 158, .height = 32, .pos = 144 * WIDTH + 97
};
static int                  drawPause = TRUE;

static EVENT                HU_Draw;

// keyboard --------------------------------------------------------------------
void (*Keyboard_Input)(int, int) = SPECTRUM_Keyboard;

// joystick --------------------------------------------------------------------
static int      joyType = JOY_CURSOR;

// programmable joystick -------------------------------------------------------
typedef struct
{
    char    *name;
    int     offset;
}
KEY;

static KEY      keyName[40] =
{
    {"0", 4}, {"1", 4}, {"2", 4}, {"3", 4}, {"4", 4}, {"5", 4}, {"6", 4}, {"7", 4}, {"8", 4}, {"9", 4},
    {"A", 4}, {"B", 4}, {"C", 4}, {"D", 4}, {"E", 4}, {"F", 4}, {"G", 4}, {"H", 4}, {"I", 4}, {"J", 4},
    {"K", 4}, {"L", 4}, {"M", 4}, {"N", 4}, {"O", 4}, {"P", 4}, {"Q", 4}, {"R", 4}, {"S", 4}, {"T", 4},
    {"U", 4}, {"V", 4}, {"W", 4}, {"X", 4}, {"Y", 4}, {"Z", 4}, {"CS", 0}, {"SS", 0}, {"Sp", 0}, {"En", 0}
};

static int      joyMap[5] = {JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_FIRE};
static char     *joyStageName[10] = {"< UP", "< DOWN", "< LEFT", "< RIGHT", "< FIRE", "     UP >", "   DOWN >", "   LEFT >", "  RIGHT >", "   FIRE >"};
static int      joyProgramChange = FALSE;
static int      joyProgramStage = 0;

// tape ------------------------------------------------------------------------
#define FREE(data)          if (data != NULL) { SYS_Free(data); data = NULL; }

#define EJECT_TAPE          TAPE_Eject(); FREE(tapeName); audioInput = 1

typedef struct stat         STATUS;

static char                 *tapeName = NULL;
static BYTE                 *tapeData = NULL;
static int                  tapeSize;
static int                  tapeReadOnly;

// help ------------------------------------------------------------------------
#define HELP(s, c)      for (arg = 0; arg < c; arg++) { SYS_Print(" %-10s - %s", s[arg].name, s[arg].description); }

#define HELP_COUNT      6
#define KEY_COUNT       16

typedef struct
{
    char    *name;
    char    *description;
}
OPTION;

static OPTION           emuHelp[HELP_COUNT] =
{
    {"-rom FILE", "48K/128K/cartridge ROM filename"},
    {"-fs", "Start fullscreen"},
    {"-ws n", "Window scale"},
    {"-keys", "Print emulator function keys"},
    {"-nopause", "Don't display PAUSED"},
    {"-vd $", "SDL video driver: wayland/x11"}
};

static OPTION           fnKey[KEY_COUNT] =
{
    {"F1", "Normal/fast tape speed"},
    {"F2", "Play/pause tape (normal speed only)"},
    {"F3", "Mute/unmute audio"},
    {"F4", "Select AY output ABC/ACB/Mono"},
    {"F5", "Reset Spectrum (press twice)"},
    {"F7", "Select joystick Sinclair/Cursor/Kempston/Fuller/Programmable"},
    {"F8", "Programmable joysticks (must be pre-selected)"},
    {"F10", "Hide/show border"},
    {"F11", "Fullscreen/window mode"},
    {"F12", "Toggle 48K/128K video mode"},
    {"End", "Tape eject (press twice)"},
    {"Home", "Rewind tape to beginning"},
    {"PageUp", "Rewind tape 1 block"},
    {"PageDown", "Fast forward tape 1 block"},
    {"Escape", "Power off (press twice)"},
    {"Pause", "Pause Spectrum (any key continues)"}
};

// system stuff ----------------------------------------------------------------
char *GetName(char *text)
{
    char    *new = strrchr(text, '/');

    if (new == NULL)
    {
        return text;
    }

    return new + 1;
}

void SYS_Print(const char *fmt, ...)
{
    va_list     list;
    char        msg[512];
    char        *text = msg;

    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);

    if (*text == '\n')
    {
        printf("\n");
        text++;
    }
    if (*text == ' ')
    {
        text++;
        printf("    ");
    }

    printf("%s\n", text);

    fflush(stdout);
}

void *SYS_Malloc(int size)
{
    return malloc(size);
}

void SYS_Free(void *ptr)
{
    free(ptr);
}

static int FILE_Check(char *name, int *size)
{
    STATUS      status;

    *size = 0;

    if (stat(name, &status) < 0 || (!S_ISREG(status.st_mode)))
    {
        return FALSE;
    }

    *size = status.st_size;
    tapeReadOnly = (status.st_mode & S_IWUSR) ? FALSE : TRUE;

    return TRUE;
}

static BYTE *FILE_Read(char *name, int size)
{
    FILE    *file;
    BYTE    *buffer = NULL;

    if ((file = fopen(name, "r")) != NULL)
    {
        buffer = SYS_Malloc(size);
        fread(buffer, 1, size, file);
        fclose(file);
    }

    return buffer;
}

void FILE_Write(BYTE *data, WORD length)
{
    FILE    *file;

    if ((file = fopen(tapeName, "a+")) != NULL)
    {
        fwrite(&length, sizeof(WORD), 1, file);
        fwrite(data, 1, length, file);
        fclose(file);
    }
}

// emulation -------------------------------------------------------------------
static void DoNothing()
{
}

static void DoEmulate()
{
    // locking the audio output seems to cause pops and clicks FIXME
    //SDL_LockAudioDevice(sdlAudioOut);
    SDL_LockTextureToSurface(texScreen, NULL, &sdlSurface);
    SPECTRUM_TVScan((Uint8 *)sdlSurface->pixels);
    HU_Draw();
    SDL_UnlockTexture(texScreen);
    //SDL_UnlockAudioDevice(sdlAudioOut);
}

static void DoQuit()
{
    emuPower = 0;
}

// headsup ---------------------------------------------------------------------
static void HU_DrawGfx(Uint8 *surface, Uint32 *gfx, HEADSUP *hu, Uint8 how[3])
{
    Uint32      line;
    Uint8       *pos;
    int         c, r;

    surface += hu->pos * BPP;

    for (c = 0; c < hu->width; c++, gfx++, surface += BPP)
    {
        line = *gfx;
        pos = surface;
        for (r = 0; r < hu->height; r++, line >>= 1, pos += WIDTH * BPP)
        {
            if (line & 1)
            {
                *(pos + 0) ^= how[0];
                *(pos + 1) ^= how[0];
                *(pos + 2) ^= how[0];
                *(pos + 0) &= how[1];
                *(pos + 1) &= how[1];
                *(pos + 2) &= how[1];
                *(pos + 0) |= how[2];
                *(pos + 1) |= how[2];
                *(pos + 2) |= how[2];
            }
        }
    }
}

static void HU_DrawText(Uint8 *surface, HEADSUP *hu, char *text)
{
    for ( ; *text; text++, surface += hu->width * BPP)
    {
        HU_DrawGfx(surface, dataCharset[*text - 32], hu, HU_GFX_INVERT);
    }
}

static void HU_DrawNormal()
{
    Uint8       *surface = (Uint8 *)sdlSurface->pixels;
    int         i, k;

    if (huJoystickTimer > 0)
    {
        huJoystickTimer--;
        HU_DrawGfx(surface, dataJoystick[joyType], &huJoystick[joyType], HU_GFX_INVERT);
    }

    if (huProgramTimer > 0)
    {
        huProgramTimer--;
        for (i = 0; i < 10; i++)
        {
            k = SPECTRUM_GetJoyKey(i / 5, joyMap[i % 5]);
            HU_DrawText(surface + keyName[k].offset * BPP, &huProgram[i], keyName[k].name);
        }

        if (joyProgramChange == TRUE)
        {
            huProgramTimer++;
            HU_DrawText(surface, &huProgram[10], joyStageName[joyProgramStage]);
        }
    }

    if (huMessageTimer > 0)
    {
        huMessageTimer--;
        HU_DrawText(surface, &huMessage, huMessageText);
    }
}

static void HU_DrawPaused()
{
    Uint8       *surface = (Uint8 *)sdlSurface->pixels;

    if (drawPause == TRUE)
    {
        HU_DrawGfx(surface, dataPaused[0], &huPaused, HU_GFX_BLACK);
        HU_DrawGfx(surface, dataPaused[1], &huPaused, HU_GFX_WHITE);
    }

    HU_Draw = HU_DrawNormal;
    Emulate = DoNothing;
}

static void HU_Message(char *text, int time)
{
    huMessageTimer = time;
    huMessage.pos = 266 * WIDTH + (VIEWWIDTH - strlen(text) * huMessage.width) / 2;
    huMessageText = text;
}

static void HU_Message2(char *text, int time)
{
    sdlLastSym = SDLK_UNKNOWN;
    HU_Message(text, time);
}

// audio -----------------------------------------------------------------------
static void Audio_Out(void *unused, Uint8 *stream, int length)
{
    (void)unused;

    short       *out = (short *)stream;

    while (length)
    {
        SPECTRUM_AudioOut(out, &vSync);
        *out++ &= audioPlay;
        *out++ &= audioPlay;

        length -= 4;
    }
}

static void Audio_In()
{
    Uint8       buffer[SAMPLES], *stream = buffer;
    int         length = 0;

    while (length < SAMPLES)
    {
        length = SDL_GetQueuedAudioSize(sdlAudioIn);
    }

    SDL_DequeueAudio(sdlAudioIn, buffer, SAMPLES);

    for (length = 0; length < SAMPLES; length++, stream++)
    {
        SPECTRUM_AudioIn((*stream & 128) ? audioInput : 0);
    }
}

// input -----------------------------------------------------------------------
static void Joystick_Program_Cancel()
{
    joyProgramChange = FALSE;
    huProgramTimer = HU_TIMEOUT;
    Keyboard_Input = SPECTRUM_Keyboard;
}

static void Joystick_Key_Input(int key, int state)
{
    if (state == 0)
    {
        return;
    }

    SPECTRUM_SetJoyKey(joyProgramStage / 5, joyMap[joyProgramStage % 5], key);

    joyProgramStage++;
    if (joyProgramStage == 10)
    {
        Joystick_Program_Cancel();
    }
}

static void Key_Input()
{
    static int      again = FALSE, twice = FALSE;
    SDL_Event       event;
    int             sym, state = 0;
    static int      pad[2] = {5, 5};
    static char     text[33];

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
          case SDL_KEYDOWN:
            state = 1;
            break;

          case SDL_KEYUP:
            state = 0;
            break;

          case SDL_MOUSEMOTION:
            mouseHideTimer = rectWindowZoom ? 0 : MOUSE_TIMEOUT;
            SDL_ShowCursor(rectWindowZoom ? SDL_FALSE : SDL_TRUE);
            continue;

          case SDL_JOYBUTTONDOWN:
          case SDL_JOYBUTTONUP:
            SPECTRUM_Joystick(event.jbutton.which, JOY_FIRE, event.jbutton.state);
            continue;

          case SDL_JOYAXISMOTION:
            if (pad[event.jaxis.axis] < 5)
            {
                SPECTRUM_Joystick(event.jbutton.which, pad[event.jaxis.axis], 0);
                pad[event.jaxis.axis] = 5;
            }
            if (event.jaxis.value < -5120 || event.jaxis.value > 5120)
            {
                if (event.jaxis.axis == 0)
                {
                    if (event.jaxis.value < 0)
                    {
                        pad[0] = JOY_LEFT;
                    }
                    else
                    {
                        pad[0] = JOY_RIGHT;
                    }
                }
                else
                {
                    if (event.jaxis.value < 0)
                    {
                        pad[1] = JOY_UP;
                    }
                    else
                    {
                        pad[1] = JOY_DOWN;
                    }
                }
                SPECTRUM_Joystick(event.jbutton.which, pad[event.jaxis.axis], 1);
            }
            continue;

          case SDL_JOYDEVICEADDED:
            SDL_JoystickOpen(event.jdevice.which);
            SYS_Print("\nPlayer %i joystick added", event.jdevice.which + 1);
            continue;

          case SDL_DROPFILE:
            EJECT_TAPE;
            if (FILE_Check(event.drop.file, &tapeSize) == TRUE)
            {
                SYS_Print("\nTape inserted");
                tapeName = strdup(event.drop.file);
                SYS_Print(" %s", GetName(tapeName));
                tapeData = FILE_Read(tapeName, tapeSize);
                TAPE_Load(tapeData, tapeSize, tapeReadOnly);
                FREE(tapeData);
                if (TAPE_Loaded() == TRUE)
                {
                    audioInput = 0;
                    SYS_Print(" %5i bytes %s", tapeSize, tapeReadOnly == TRUE ? "READ ONLY" : "");
                    HU_Message2(tapeReadOnly == TRUE ? "Tape inserted READ ONLY" : "Tape inserted", HU_TIMEOUT);
                }
                else
                {
                    EJECT_TAPE;
                    HU_Message2("Tape has no valid data!", HU_TIMEOUT);
                }
            }
            else
            {
                HU_Message2("Check tape is inserted correctly!", HU_TIMEOUT);
            }
            SDL_free(event.drop.file);
            continue;

          case SDL_QUIT:
            DoQuit();
            continue;

          default:
            continue;
        }

        sym = event.key.keysym.sym;

        if (emuPaused && state == 1 && sym != SDLK_PAUSE && sym != SDLK_F10 && sym != SDLK_F11)
        {
            emuPaused = FALSE;
            Emulate = DoEmulate;
            continue;
        }

        switch (sym)
        {
          case SDLK_1:
          case SDLK_2:
          case SDLK_3:
          case SDLK_4:
          case SDLK_5:
          case SDLK_6:
          case SDLK_7:
          case SDLK_8:
          case SDLK_9:
          case SDLK_0:
            Keyboard_Input(sym - SDLK_0 + KEY_0, state);
            break;

          case SDLK_a:
          case SDLK_b:
          case SDLK_c:
          case SDLK_d:
          case SDLK_e:
          case SDLK_f:
          case SDLK_g:
          case SDLK_h:
          case SDLK_i:
          case SDLK_j:
          case SDLK_k:
          case SDLK_l:
          case SDLK_m:
          case SDLK_n:
          case SDLK_o:
          case SDLK_p:
          case SDLK_q:
          case SDLK_r:
          case SDLK_s:
          case SDLK_t:
          case SDLK_u:
          case SDLK_v:
          case SDLK_w:
          case SDLK_x:
          case SDLK_y:
          case SDLK_z:
            Keyboard_Input(sym - SDLK_a + KEY_A, state);
            break;

          case SDLK_LSHIFT:
          case SDLK_RSHIFT:
            Keyboard_Input(KEY_CS, state);
            break;

          case SDLK_LCTRL:
          case SDLK_RCTRL:
            Keyboard_Input(KEY_SS, state);
            break;

          case SDLK_RETURN:
            Keyboard_Input(KEY_EN, state);
            break;

          case SDLK_SPACE:
            Keyboard_Input(KEY_SP, state);
            break;
        }

        if (joyProgramChange == TRUE)
        {
            if (sym == SDLK_ESCAPE)
            {
                Joystick_Program_Cancel();
            }
            continue;
        }

        switch (sym)
        {
          case SDLK_RIGHT:
            SPECTRUM_Joystick(0, JOY_RIGHT, state);
            break;

          case SDLK_LEFT:
            SPECTRUM_Joystick(0, JOY_LEFT, state);
            break;

          case SDLK_DOWN:
            SPECTRUM_Joystick(0, JOY_DOWN, state);
            break;

          case SDLK_UP:
            SPECTRUM_Joystick(0, JOY_UP, state);
            break;

          case SDLK_TAB:
            SPECTRUM_Joystick(0, JOY_FIRE, state);
            break;
        }

        if (state == 0)
        {
            continue;
        }

        if (sdlLastSym == sym && huMessageTimer > 0)
        {
            again = TRUE;
        }
        else
        {
            again = FALSE;
        }

        switch (sym)
        {
          case SDLK_ESCAPE:
            if (again == FALSE)
            {
                HU_Message("Press ESC again to Power Off", HU_TIMEOUT_KEY);
                twice = TRUE;
            }
            else
            {
                DoQuit();
            }
            break;

          case SDLK_F1:
            if (TAPE_Loaded() == TRUE)
            {
                if (again == TRUE)
                {
                    TAPE_SpeedToggle();
                }
                HU_Message(TAPE_FastSpeed() == TRUE ? "Fast tape speed" : "Normal tape speed", HU_TIMEOUT);
            }
            break;

          case SDLK_F2:
            if (TAPE_Loaded() == TRUE)
            {
                if (TAPE_Ended() == TRUE)
                {
                    HU_Message("Tape ended", HU_TIMEOUT);
                }
                else
                {
                    HU_Message(TAPE_Playing() == TRUE ? "Tape playing" : "Tape paused", HU_TIMEOUT);
                }
            }
            break;

          case SDLK_F3:
            audioPlay ^= 0xffff;
            HU_Message(audioPlay == 0xffff ? "Audio playing" : "Audio muted", HU_TIMEOUT);
            break;

          case SDLK_F4:
            if (again == TRUE)
            {
                if (audioSetup == AY_ABC)
                {
                    audioSetup = AY_ACB;
                    audioText = TEXT_AY_ACB;
                    SPECTRUM_AudioSetup(AY_ACB);
                }
                else if (audioSetup == AY_ACB)
                {
                    audioSetup = AY_MONO;
                    audioText = TEXT_AY_MONO;
                    SPECTRUM_AudioSetup(AY_MONO);
                }
                else if (audioSetup == AY_MONO)
                {
                    audioSetup = AY_ABC;
                    audioText = TEXT_AY_ABC;
                    SPECTRUM_AudioSetup(AY_ABC);
                }
            }
            HU_Message(audioText, HU_TIMEOUT);
            break;

          case SDLK_F5:
            if (again == FALSE)
            {
                HU_Message("Press F5 again to Reset", HU_TIMEOUT_KEY);
                twice = TRUE;
            }
            else
            {
                SPECTRUM_Reset();
                huMessageTimer = 0;
            }
            break;

          case SDLK_F7:
            if (huJoystickTimer > 0)
            {
                if (joyType == JOY_SINCLAIR)
                {
                    joyType = JOY_CURSOR;
                }
                else if (joyType == JOY_CURSOR)
                {
                    joyType = JOY_KEMPSTON;
                }
                else if (joyType == JOY_KEMPSTON)
                {
                    joyType = JOY_FULLER;
                }
                else if (joyType == JOY_FULLER)
                {
                    joyType = JOY_PROGRAM;
                }
                else if (joyType == JOY_PROGRAM)
                {
                    joyType = JOY_SINCLAIR;
                }
                SPECTRUM_JoystickSelect(joyType);
            }
            huJoystickTimer = HU_TIMEOUT;
            huProgramTimer = 0;
            break;

          case SDLK_F8:
            if (huJoystickTimer > 0 && joyType == JOY_PROGRAM && rectScreenZoom == FALSE)
            {
                huProgramTimer = HU_TIMEOUT;
                huJoystickTimer = 0;
                joyProgramChange = TRUE;
                joyProgramStage = 0;
                Keyboard_Input = Joystick_Key_Input;
            }
            break;

          case SDLK_F10:
            rectScreenZoom ^= 1;
            if (emuPaused == FALSE)
            {
                if (rectScreenZoom == TRUE)
                {
                    HU_Draw = DoNothing;
                }
                else
                {
                    HU_Draw = HU_DrawNormal;
                }
            }
            break;

          case SDLK_F11:
            rectWindowZoom ^= 1;
            FULLSCREEN;
            break;

          case SDLK_F12:
            if (again == TRUE)
            {
                if (ulaType == ULA_48K)
                {
                    ulaType = ULA_128K;
                    ulaText = TEXT_ULA_128K;
                }
                else if (ulaType == ULA_128K)
                {
                    ulaType = ULA_48K;
                    ulaText = TEXT_ULA_48K;
                }
                SPECTRUM_UlaType(ulaType);
            }
            HU_Message(ulaText, HU_TIMEOUT);
            break;

          case SDLK_END:
            if (TAPE_Loaded() == TRUE)
            {
                if (again == FALSE)
                {
                    HU_Message("Press again to eject tape", HU_TIMEOUT_KEY);
                    twice = TRUE;
                }
                else
                {
                    EJECT_TAPE;
                    HU_Message("Tape ejected", HU_TIMEOUT);
                }
            }
            break;

          case SDLK_HOME:
            if (TAPE_Loaded() == TRUE)
            {
                TAPE_Rewind(0);
                HU_Message("Tape rewound to beginning", HU_TIMEOUT_KEY);
            }
            break;

          case SDLK_PAGEUP:
            if (TAPE_Loaded() == TRUE)
            {
                sprintf(text, "Tape rewound to block %i of %i", TAPE_Rewind(1), TAPE_BlockCount());
                HU_Message(text, HU_TIMEOUT_KEY);
            }
            break;

          case SDLK_PAGEDOWN:
            if (TAPE_Loaded() == TRUE)
            {
                sprintf(text, "Tape advanced to block %i of %i", TAPE_FastForward(), TAPE_BlockCount());
                HU_Message(text, HU_TIMEOUT_KEY);
            }
            break;

          case SDLK_PAUSE:
            if (emuPaused == FALSE)
            {
                emuPaused = TRUE;
                HU_Draw = HU_DrawPaused;
                if (twice == TRUE) // some messages must be cancelled
                {
                    huMessageTimer = 0;
                    twice = FALSE;
                }
            }
            else
            {
                emuPaused = FALSE;
                Emulate = DoEmulate;
            }
            break;
        }

        if (sym != SDLK_F8 && sym != SDLK_F9)
            sdlLastSym = sym;
    }
}

// window ----------------------------------------------------------------------
SDL_HitTestResult WindowDrag(SDL_Window *window, const SDL_Point *point, void *unused)
{
    (void)window, (void)point, (void)unused;

    return SDL_HITTEST_DRAGGABLE;
}

// where it all begins ---------------------------------------------------------
int main(int argc, char **argv)
{
    SDL_DisplayMode     mode;
    SDL_AudioSpec       want;
    int                 scale = 2;      // initial scale of window
    int                 arg;
    int                 help = 0;
    char                *file = "NONE"; // rom file
    BYTE                *rom;           // rom data
    int                 size = 0;       // rom size
    char                *driver = "";

    SYS_Print(TITLE" v1.0.1 ("__DATE__")");

    for (arg = 1; arg < argc; arg++)
    {
        if (arg < argc - 1 && strcmp(argv[arg], "-rom") == 0)
        {
            arg++;

            if (size > 0)
            {
                SYS_Print("\n Too many ROMs specified");
                help = 1;
                break;
            }
            if (FILE_Check(argv[arg], &size) == TRUE)
            {
                if (size == 16384 || size == 32768)
                {
                    file = argv[arg];
                }
                else
                {
                    SYS_Print("\n %s", argv[arg]);
                    SYS_Print(" ROM size incorrect: %i", size);
                    return 2;
                }
            }
            else
            {
                SYS_Print("\n %s", argv[arg]);
                SYS_Print(" Not a file!");
                return 2;
            }
        }
        else if (strcmp(argv[arg], "-fs") == 0)
        {
            rectWindowZoom = 1;
        }
        else if (arg < argc - 1 && strcmp(argv[arg], "-ws") == 0)
        {
            arg++;
            scale = atoi(argv[arg]);
            if (scale < 0)
            {
                scale = 1;
            }
        }
        else if (strcmp(argv[arg], "-keys") == 0)
        {
            help = 2;
        }
        else if (strcmp(argv[arg], "-nopause") == 0)
        {
            drawPause = FALSE;
        }
        else if (strcmp(argv[arg], "-vd") == 0 && arg < argc - 1)
       {
            arg++;
            driver = argv[arg];
        }
        else
        {
            help = 1;
        }
    }

    if (help == 2)
    {
        SYS_Print("\nEmulator keys:");
        HELP(fnKey, KEY_COUNT);
    }

    if (help == 1 || size == 0)
    {
        SYS_Print("\nCommand line options:");
        HELP(emuHelp, HELP_COUNT);
        return 1;
    }

    SDL_SetHint(SDL_HINT_APP_NAME, TITLE);
    SDL_SetHint(SDL_HINT_VIDEODRIVER, driver);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);

    rectTarget[0][0].w *= scale;
    rectTarget[0][0].h *= scale;
    rectTarget[0][1].w *= scale;
    rectTarget[0][1].h *= scale;

    rectWindow[0].w *= scale;
    rectWindow[0].h *= scale;

    SDL_GetDesktopDisplayMode(0, &mode);

    if (mode.h * 4 / 3 <= mode.w)
    {
        // landscape
        rectWindow[1].h = mode.h;
        rectWindow[1].w = rectWindow[1].h * 4 / 3;
        rectWindow[1].x = (mode.w - rectWindow[1].w) / 2;
    }
    else
    {
        // portrait
        rectWindow[1].w = mode.w;
        rectWindow[1].h = rectWindow[1].w * 3 / 4;
        rectWindow[1].y = (mode.h - rectWindow[1].h) / 2;
    }

    scale = rectWindow[1].w / VIEWWIDTH;

    rectTarget[1][0].w *= scale;
    rectTarget[1][0].h *= scale;
    rectTarget[1][1].w *= scale;
    rectTarget[1][1].h *= scale;

    if (rectWindow[0].w > mode.w || rectWindow[0].h > mode.h)
    {
        SYS_Print("\nWindow scale multiplier exceeds display size!");
        SYS_Print(" Display: %i x %i", mode.w, mode.h);
        SYS_Print(" Window:  %i x %i", rectWindow[0].w, rectWindow[0].h);
        SDL_Quit();
        return 1;
    }

    rom = FILE_Read(file, size);
    ROM_Load(rom, size);
    SYS_Free(rom);
    SYS_Print("\nLoaded ROM: %s", GetName(file));
    SYS_Print(" Size: %iK", size / 1024);
    if (size == 32768)
    {
        ulaType = ULA_128K;
        ulaText = TEXT_ULA_128K;
    }
    SYS_Print("\nStarted in %s", ulaText);
    TAPE_RomLock(size);

    signal(SIGINT, DoQuit);

    SDL_JoystickEventState(SDL_ENABLE);

    sdlWindow = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, rectWindow[0].w, rectWindow[0].h, SDL_WINDOW_BORDERLESS);
    FULLSCREEN;

    SDL_SetWindowHitTest(sdlWindow, WindowDrag, NULL);

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    texTarget = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, rectTarget[1][0].w, rectTarget[1][0].h);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    texScreen = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    want.freq = SAMPLERATE;
    want.format = AUDIO_S16SYS;
    want.samples = 512; // must be smaller than SAMPLERATE / 50 = 882
    want.channels = 2;
    want.callback = Audio_Out;

    sdlAudioOut = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);

    want.format = AUDIO_S8;
    want.channels = 1;
    want.callback = NULL;

    sdlAudioIn = SDL_OpenAudioDevice(NULL, 1, &want, NULL, 0);

    SDL_PauseAudioDevice(sdlAudioOut, SDL_FALSE);
    SDL_PauseAudioDevice(sdlAudioIn, SDL_FALSE);

    HU_Draw = HU_DrawNormal;
    Emulate = DoEmulate;
    SPECTRUM_UlaType(ulaType);
    SPECTRUM_JoystickSelect(joyType);
    SPECTRUM_AudioSetup(audioSetup);

    while (emuPower)
    {
        Key_Input();
        Audio_In();
        Emulate();

        SDL_RenderClear(sdlRenderer);
        SDL_SetRenderTarget(sdlRenderer, texTarget);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, texScreen, &rectScreen[rectScreenZoom], &rectTarget[rectWindowZoom][rectScreenZoom]);
        SDL_SetRenderTarget(sdlRenderer, NULL);
        SDL_RenderCopy(sdlRenderer, texTarget, &rectTarget[rectWindowZoom][rectScreenZoom], &rectWindow[rectWindowZoom]);

        do
        {
            SDL_Delay(1);
        }
        while (vSync == FALSE);
        vSync = FALSE;

        SDL_RenderPresent(sdlRenderer);

        if (mouseHideTimer > 0)
        {
            mouseHideTimer--;
            if (mouseHideTimer == 0)
            {
                SDL_ShowCursor(SDL_DISABLE);
            }
        }
    }

    EJECT_TAPE;

    SDL_CloseAudioDevice(sdlAudioOut);
    SDL_CloseAudioDevice(sdlAudioIn);

    SDL_DestroyTexture(texScreen);
    SDL_DestroyTexture(texTarget);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);

    SDL_Quit();

    SYS_Print("\nGoodbye.");

    return 0;
}
