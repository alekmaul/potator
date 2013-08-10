#ifndef __SOUND_H__
#define __SOUND_H__

#include "supervision.h"

#define BPS			22050
#define BPSMAX		44100
#define BPSMIN		11025
#define BUFSIZE		BPS
#define POFF		128
#define PDIV		3
#define PH			POFF+PDIV*8
#define PL			POFF-PDIV*7

extern uint8       noise_buffer[BUFSIZE];
extern uint8       voice_buffer[BUFSIZE];
extern uint8       dma_buffer[BUFSIZE];
extern uint8       final_buffer[BUFSIZE];

void sound_init();
void sound_reset();
void sound_done();
void sound_write(uint32 Addr, uint8 data);
void sound_noise_write(uint32 Addr, uint8 data);
void sound_audio_dma(uint32 Addr, uint8 data);
void sound_exec(uint32 cycles);
void audio_turnSound(BOOL bOn);

#endif
