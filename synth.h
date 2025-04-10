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
    float oscMix;

    float last_sample;
    float lowpass_cutoff;
    float lowpass_alpha;

    pthread_mutex_t lock;
} SynthData;

int start_audio(SynthData *data, PaStream **stream);
void stop_audio(PaStream *stream);
float lowpass_filter(float input, SynthData *data);
void update_lowpass_alpha(SynthData *data);
#endif
