#ifndef SYNTH_H
#define SYNTH_H

#include <portaudio.h>
#include <pthread.h>
#include "waveform.h"

#define SAMPLE_RATE 44100
#define AMPLITUDE 0.25
#define TWO_PI (3.14159265 * 2)
#define FRAMES_PER_BUFFER 64

typedef struct {
    float frequency;
    float phase;
    WaveformType waveform;
} Oscillator;

typedef struct {
    Oscillator osc1;
    Oscillator osc2;
    float oscMix; // Span: 0-1

    float last_sample;
    float LP_cutoff;
    float LP_alpha;

    pthread_mutex_t lock;
} SynthData;

int start_audio(SynthData *data, PaStream **stream);
void stop_audio(PaStream *stream);

#endif
