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
    float amplitude;
    float osc2Detune;
    float oscMix;

    float lowpass_stage1;
    float lowpass_stage2;
    float lowpass_last_sample; // Not needed?
    float lowpass_alpha;
    float lowpass_cutoff;
    float lowpass_resonance;
    float lowpass_feedback;

    float highpass_prev_input;
    float highpass_prev_output;
    float highpass_cutoff;
    float highpass_alpha;
    
    pthread_mutex_t lock;
} SynthData;

int start_audio(SynthData *data, PaStream **stream);
void stop_audio(PaStream *stream);

float lowpass_two_stage(float input, SynthData *data);
void update_lowpass_alpha(SynthData *data);

float highpass_filter(float input, SynthData *data);
void update_highpass_alpha(SynthData *data);

float saturate(float input);
#endif
