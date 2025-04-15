/* A little synth project */


#include "synth.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

float saturate(float input) {
    return tanh(input);
}

float lowpass_two_stage(float input, SynthData *data) {
    // Saturate the resonance feedback
    float feedback = data->lowpass_resonance * saturate(data->lowpass_stage2);
    float inputWithFeedback = input - feedback;
    // First stage
    data->lowpass_stage1 += data->lowpass_alpha * (inputWithFeedback - data->lowpass_stage1);
    // Second stage
    data->lowpass_stage2 += data->lowpass_alpha * (data->lowpass_stage1 - data->lowpass_stage2);

    return data->lowpass_stage2;
}

void update_lowpass_alpha(SynthData *data) {
    float rc = 1.0f / (2.0f * M_PI * data->lowpass_cutoff);
    float dt = 1.0f / SAMPLE_RATE;
    data->lowpass_alpha = dt / (rc + dt);
}

// TODO figure out why adding resonance broke the LP
float highpass_filter(float input, SynthData *data) {
    float output = data->highpass_alpha * (data->highpass_prev_output + input - data->highpass_prev_input);
    data->highpass_prev_input = input;
    data->highpass_prev_output = output;
    return output;
}

void update_highpass_alpha(SynthData *data) {
    float rc = 1.0f / (2.0f * M_PI * data->highpass_cutoff);
    float dt = 1.0f / SAMPLE_RATE;
    data->highpass_alpha = rc / (rc + dt);
}

static int audioCallback(
    const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
) {
    (void)inputBuffer;
    (void)timeInfo;
    (void)statusFlags;

    // OutputBuffer is a buffer of audio samples created by PortAudio lib. 
    // Looks like this? [ left0, right0, left1, right1, left2, right2, ... ]
    // Where each entry is a 32 bit float
    float *out = (float*)outputBuffer;
    float amplitude;
    float freqOsc1, freqOsc2, wave1, wave2;
    float sampleOsc1, sampleOsc2;
    float mix, mixFiltered;
    
    SynthData *data = (SynthData*)userData;

    // Lock the mutex to safely access shared data
    pthread_mutex_lock(&data->lock);
    amplitude = data->amplitude;
    freqOsc1 = data->osc1.frequency;  // Frequency of the first oscillator
    freqOsc2 = data->osc2.frequency;  // Frequency of the second oscillator
    wave1 = data->osc1.waveform;
    wave2 = data->osc2.waveform;
    mix = data->oscMix;
    pthread_mutex_unlock(&data->lock);

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        sampleOsc1 = amplitude * generate_sample(wave1, data->osc1.phase);
        sampleOsc2 = amplitude * generate_sample(wave2, data->osc2.phase);

        // Mix oscillators
        float mixedSample = (mix * sampleOsc1) + ((1.0f - mix) * sampleOsc2);

        // Apply filters
        float lpFiltered = lowpass_two_stage(mixedSample, data);
        mixFiltered = highpass_filter(lpFiltered, data);

        *out++ = mixFiltered; // left channel
        *out++ = mixFiltered; // right channel
        
        // Update phase
        data->osc1.phase += (float)(TWO_PI * freqOsc1 / SAMPLE_RATE);
        if (data->osc1.phase >= TWO_PI) data->osc1.phase -= TWO_PI;

        data->osc2.phase += (float)(TWO_PI * freqOsc2 / SAMPLE_RATE);
        if (data->osc2.phase >= TWO_PI) data->osc2.phase -= TWO_PI;
    }

    return paContinue;
}

int start_audio(SynthData *data, PaStream **stream) {
    pthread_mutex_init(&data->lock, NULL);
    data->lowpass_last_sample = 0.0f;
    update_lowpass_alpha(data);
    Pa_Initialize();
    Pa_OpenDefaultStream(stream,
                         0,
                         2,
                         paFloat32,
                         SAMPLE_RATE,
                         FRAMES_PER_BUFFER,
                         audioCallback,
                         data);

    return Pa_StartStream(*stream);
}

void stop_audio(PaStream *stream) {
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}

