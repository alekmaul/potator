#include "shared.h"

SDL_AudioSpec spec;
SDL_AudioSpec obtained, retSpec;
SDL_mutex *sndlock;

void Acallback(void *userdata, Uint8 *stream, int len) {
	unsigned short c;
	signed short s;
	unsigned int i;

	SDL_mutexP(sndlock);
	sound_stream_update(stream,len);
	SDL_mutexV(sndlock);
}

void Ainit(void) {
	int i;
	
	// Init sound
	spec.freq = BPS;
	spec.format = AUDIO_S16SYS; //AUDIO_U8;
	spec.channels = 2;
	spec.samples = 1024;
	spec.callback = Acallback;
	spec.userdata = NULL;

    // Open the audio device and start playing sound! 
    if ( SDL_OpenAudio(&spec, &retSpec) < 0 ) {
        fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
        exit(1);
    }

	sndlock = SDL_CreateMutex();
	if (sndlock == NULL) {
		fprintf(stderr, "Unable to create lock: %s\n", SDL_GetError());
		SDL_CloseAudio();
        exit(1);
	}
}

void Aclose(void) {
	SDL_PauseAudio(1);
	SDL_DestroyMutex(sndlock);

	SDL_CloseAudio();
}
