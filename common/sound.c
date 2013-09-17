////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////
#include "sound.h"
#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL/SDL.h>

typedef struct {
	unsigned char reg[4];
	int on;
	int waveform, volume;
	int pos;
	int size;
	int count;
} SVISION_CHANNEL;
SVISION_CHANNEL m_channel[2];

typedef enum  {
	SVISION_NOISE_Type7Bit,
	SVISION_NOISE_Type14Bit
} SVISION_NOISE_Type;

typedef struct  {
	unsigned char reg[3];
	int on, right, left, play;
	SVISION_NOISE_Type type;
	int state;
	int volume;
	int count;
	double step, pos;
	int value; // currently simple random function
} SVISION_NOISE;
SVISION_NOISE m_noise;

typedef struct  {
	unsigned char reg[5];
	int on, right, left;
	int ca14to16;
	int start,size;
	double pos, step;
	int finished;
} SVISION_DMA;
SVISION_DMA m_dma;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void sound_stream_update(Uint8 *stream, int len) {
	int i, j;
	SVISION_CHANNEL *channel;
	short s;
	unsigned short *left=(unsigned short *) stream;
	unsigned short *right=(unsigned short *) (stream+1);

	for (i = 0; i < len>>1; i++) {
		s=0;

		for (channel=m_channel, j=0; j<2; j++, channel++) {
			if (channel->size != 0) {
				if (channel->on||channel->count) {
					int on = FALSE;
					switch (channel->waveform) {
						case 0:
							on = channel->pos <= (28 * channel->size) >> 5;
							break;
						case 1:
							on = channel->pos <= (24 * channel->size) >> 5;
							break;
						default:
						case 2:
							on = channel->pos <= channel->size / 2;
							break;
						case 3:
							on = channel->pos <= (9 * channel->size) >> 5;
							break;
					}
					//s += on ? channel->volume<<8 : 0;
					s = on ? channel->volume << 8 : 0;
					if (j == 0) {
						*right += s; 
					}
					else {
						*left += s;
					}
				}
				channel->pos++;
				if (channel->pos >= channel->size)
					channel->pos = 0;
			}
		}

		if (m_noise.on && (m_noise.play || m_noise.count)) {
			s = (m_noise.value ? 1 << 8: 0) * m_noise.volume;
			int b1, b2;
			if (m_noise.left)
				*left += s;
			if (m_noise.right)
				*right += s;
			m_noise.pos += m_noise.step;
			if (m_noise.pos >= 1.0) {
				switch (m_noise.type) {
					case SVISION_NOISE_Type7Bit:
						m_noise.value = m_noise.state & 0x40 ? 1 : 0;
						b1 = (m_noise.state & 0x40) != 0;
						b2 = (m_noise.state & 0x20) != 0;
						m_noise.state=(m_noise.state<<1)+(b1!=b2?1:0);
						break;
					case SVISION_NOISE_Type14Bit:
					default:
						m_noise.value = m_noise.state & 0x2000 ? 1 : 0;
						b1 = (m_noise.state & 0x2000) != 0;
						b2 = (m_noise.state & 0x1000) != 0;
						m_noise.state = (m_noise.state << 1) + (b1 != b2 ? 1 : 0);
				}
				m_noise.pos -= 1;
			}
		}

		if (m_dma.on) {
			unsigned char sample;
			unsigned short addr = m_dma.start + (unsigned) m_dma.pos / 2;
			if (addr >= 0x8000 && addr < 0xc000) {
				sample = Rd6502( (addr & 0x3fff) | m_dma.ca14to16);
				//sample = machine().root_device().memregion("user1")->base()[(addr & 0x3fff) | m_dma.ca14to16];
			}
			else {
				sample = Rd6502(addr);
				//sample = machine().device("maincpu")->memory().space(AS_PROGRAM).read_byte(addr);
			}
			if (((unsigned)m_dma.pos) & 1)
				s = (sample & 0xf);
			else
				s = (sample & 0xf0) >> 4;
			s <<= 8;
			if (m_dma.left)
				*left += s;
			if (m_dma.right)
				*right += s;
			m_dma.pos += m_dma.step;
			if (m_dma.pos >= m_dma.size) {
				//svision_state *sv_state = machine().driver_data<svision_state>();
				m_dma.finished = TRUE;
				m_dma.on = FALSE;
				interrupts_irq();
				//sv_state->svision_irq();
			}
		}
		
		right++; left++;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void sound_decrement() {
	if (m_channel[0].count > 0)
		m_channel[0].count--;
	if (m_channel[1].count > 0)
		m_channel[1].count--;
	if (m_noise.count > 0)
		m_noise.count--;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void soundport_w(int which, int offset, int data) {
	SVISION_CHANNEL *channel = &m_channel[which];
	unsigned short size;

	//m_mixer_channel->update();
	channel->reg[offset] = data;

	switch (offset)
	{
		case 0:
		case 1:
			size = channel->reg[0] | ((channel->reg[1] & 7) << 8);
			if (size)
			{
				//  channel->size=(int)(device->machine().sample_rate()*(size<<5)/4e6);
				channel->size= (int) (BPS * (size << 5) / 4000000);
			}
			else
			{
				channel->size = 0;
			}
			channel->pos = 0;
			break;
		case 2:
			channel->on = data & 0x40;
			channel->waveform = (data & 0x30) >> 4;
			channel->volume = data & 0xf;
			break;
		case 3:
			channel->count = data + 1;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void svision_sounddma_w(int offset, int data) {
//	logerror("%.6f svision snddma write %04x %02x\n", space.machine().time().as_double(),offset+0x18,data);
	m_dma.reg[offset] = data;
	switch (offset)	{
		case 0:
		case 1:
			m_dma.start = (m_dma.reg[0] | (m_dma.reg[1] << 8));
			break;
		case 2:
			m_dma.size = (data ? data : 0x100) * 32;
			break;
		case 3:
			m_dma.step = 4000000 / (256.0 * BPS * (1 + (data & 3)));
			m_dma.right = data & 4;
			m_dma.left = data & 8;
			m_dma.ca14to16 = ((data & 0x70) >> 4) << 14;
			break;
		case 4:
			m_dma.on = data & 0x80;
			if (m_dma.on) {
				m_dma.pos = 0.0;
			}
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void  svision_noise_w(int offset, int data) {
	//  logerror("%.6f svision noise write %04x %02x\n",machine.time(),offset+0x28,data);
	m_noise.reg[offset]=data;
	switch (offset) {
		case 0:
			m_noise.volume=data&0xf;
			m_noise.step= 4000000 / (256.0*BPS*(1+(data>>4)));
			break;
		case 1:
			m_noise.count = data + 1;
			break;
		case 2:
			m_noise.type = (SVISION_NOISE_Type) (data & 1);
			m_noise.play = data & 2;
			m_noise.right = data & 4;
			m_noise.left = data & 8;
			m_noise.on = data & 0x10; /* honey bee start */
			m_noise.state = 1;
			break;
	}
	m_noise.pos=0.0;
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void sound_init(vid) {
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void sound_reset(void) {
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void sound_done(void) {
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void audio_turnSound(int bOn) {
}
