#ifndef __SPECTRUM__
#define __SPECTRUM__

#include "types.h"

#define FALSE           0
#define TRUE            1

// video
#define WIDTH           456
#define HEIGHT          312

#define BPP             3       // bytes per pixel

#define FPS             50

#define ULA_48K         0
#define ULA_128K        1

// audio
#define SAMPLERATE      44100   // DO NOT CHANGE!
#define SAMPLES         882     // per frame

#define SECOND          3500000 // tape speed

#define AY_ABC          0
#define AY_ACB          1
#define AY_MONO         2

// supported joysticks
#define JOY_SINCLAIR    0
#define JOY_CURSOR      1
#define JOY_KEMPSTON    2
#define JOY_FULLER      3
#define JOY_PROGRAM     4

// joystick mapping
#define JOY_RIGHT       0
#define JOY_LEFT        1
#define JOY_DOWN        2
#define JOY_UP          3
#define JOY_FIRE        4

// keyboard mapping
#define KEY_0           0
#define KEY_1           1
#define KEY_2           2
#define KEY_3           3
#define KEY_4           4
#define KEY_5           5
#define KEY_6           6
#define KEY_7           7
#define KEY_8           8
#define KEY_9           9
#define KEY_A           10
#define KEY_B           11
#define KEY_C           12
#define KEY_D           13
#define KEY_E           14
#define KEY_F           15
#define KEY_G           16
#define KEY_H           17
#define KEY_I           18
#define KEY_J           19
#define KEY_K           20
#define KEY_L           21
#define KEY_M           22
#define KEY_N           23
#define KEY_O           24
#define KEY_P           25
#define KEY_Q           26
#define KEY_R           27
#define KEY_S           28
#define KEY_T           29
#define KEY_U           30
#define KEY_V           31
#define KEY_W           32
#define KEY_X           33
#define KEY_Y           34
#define KEY_Z           35
#define KEY_CS          36
#define KEY_SS          37
#define KEY_SP          38
#define KEY_EN          39

extern void (*SPECTRUM_Joystick)(int, int, int);

void SPECTRUM_TVScan(BYTE *);
void SPECTRUM_Reset(void);
void SPECTRUM_AudioOut(short [2], int *);
void SPECTRUM_AudioIn(short);
void SPECTRUM_AudioSetup(int);
void SPECTRUM_UlaType(int);
void SPECTRUM_Keyboard(int, int);
void SPECTRUM_JoystickSelect(int);
int SPECTRUM_GetJoyKey(int, int);
void SPECTRUM_SetJoyKey(int, int, int);

// MEMORY
void ROM_Load(BYTE *, int);

// tape
void TAPE_Load(BYTE *, int, int);
int TAPE_BlockCount(void);
int TAPE_Playing(void);
int TAPE_Rewind(int);
int TAPE_FastForward(void);
void TAPE_Eject(void);
void TAPE_SpeedToggle(void);
int TAPE_FastSpeed(void);
int TAPE_Ended(void);
int TAPE_Loaded(void);

// these functions are supplied by main.c
void FILE_Write(BYTE *, WORD);

void SYS_Print(const char *, ...);
void *SYS_Malloc(int);
void SYS_Free(void *);

#endif
