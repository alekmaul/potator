#ifndef SHARED_H
#define SHARED_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

// defines and macros
#define MAX__PATH 1024
#define FILE_LIST_ROWS 19

#define SYSVID_WIDTH	160
#define SYSVID_HEIGHT	160

#define GF_GAMEINIT    1
#define GF_MAINUI      2
#define GF_GAMEQUIT    3
#define GF_GAMERUNNING 4

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define true 1
#define false 0
typedef int bool;

#define PIX_TO_RGB(fmt, r, g, b) (((r>>fmt->Rloss)<<fmt->Rshift)| ((g>>fmt->Gloss)<<fmt->Gshift)|((b>>fmt->Bloss)<<fmt->Bshift))

// potator dependencies
#include "../../common/sound.h"
#include "../../common/memorymap.h"
#include "../../common/supervision.h"
#include "../../common/controls.h"
#include "../../common/types.h"

extern unsigned char controls_state;

#define cartridge_IsLoaded() (strlen(gameName) != 0)

typedef struct {
  unsigned int sndLevel;
  unsigned int m_ScreenRatio; // 0 = original show, 1 = full screen
  unsigned int OD_Joy[12]; // each key mapping
  unsigned int m_DisplayFPS;
  char current_dir_rom[MAX__PATH];
  unsigned int m_Color;
} gamecfg;

extern char gameName[MAX__PATH];
extern unsigned int gameCRC;

//typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int uint;

extern SDL_Surface* screen;						// Main program screen
extern SDL_Surface* actualScreen;						// Main program screen

extern SDL_Surface *layer,*layerback,*layerbackgrey;

extern SDL_Event event;

extern gamecfg GameConf;
extern unsigned int m_Flag;

extern char current_conf_app[MAX__PATH];

extern void system_loadcfg(char *cfg_name);
extern void system_savecfg(char *cfg_name);

extern unsigned long crc32 (unsigned int crc, const unsigned char *buf, unsigned int len);

extern void mainemuinit();

// menu
extern void screen_showtopmenu(void);
extern void print_string_video(int x, int y, const char *s);
extern void gethomedir(char *dir, char* name);

// sound
extern void AWriteAudioData(unsigned int chan, unsigned int len, unsigned char *pWav);
extern void Ainit(void);
extern void Aclose(void);
extern void APlayVoice(unsigned int chan, unsigned char *pWav, unsigned int loop);
extern void AStopVoice(unsigned int chan, unsigned char *pWav);

#endif
