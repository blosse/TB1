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

float highpass_filter(SynthData *data, float input) {
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
    int currentNote;
    
    AudioData *audioData = (AudioData*)userData;
    SynthData *synthData = &audioData->synthData;
    ArpData *arpData = &audioData->arpData;
    // Lock the mutex to safely access shared data
    pthread_mutex_lock(&synthData->lock);
    amplitude = synthData->amplitude;
    wave1 = synthData->osc1.waveform;
    wave2 = synthData->osc2.waveform;
    mix = synthData->oscMix;
    pthread_mutex_unlock(&synthData->lock);

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        // Updating the frequency each frame now, maybe not necessary
        // Can be improved in future if performance is an issue
        update_arp(arpData);
        currentNote = get_arp_note(arpData);
        freqOsc1 = calculate_frequency(currentNote, 0);
        freqOsc2 = calculate_frequency(currentNote, synthData->osc2Detune);
        // if (i == 0) printf("Note: %d, OSC1: %f, OSC2: %f \n", currentNote, freqOsc1, freqOsc2);

        pthread_mutex_lock(&synthData->lock);
        synthData->osc1.frequency = freqOsc1; // Frequency of the fst oscillator
        synthData->osc2.frequency = freqOsc2; // Frequency of the snd oscillator
        pthread_mutex_unlock(&synthData->lock);

        sampleOsc1 = amplitude * generate_sample(wave1, synthData->osc1.phase);
        sampleOsc2 = amplitude * generate_sample(wave2, synthData->osc2.phase);

        // Mix oscillators
        float mixedSample = (mix * sampleOsc1) + ((1.0f - mix) * sampleOsc2);

        // Apply filters
        float lpFiltered = lowpass_two_stage(mixedSample, synthData);
        mixFiltered = highpass_filter(synthData, lpFiltered);

        *out++ = mixFiltered; // left channel
        *out++ = mixFiltered; // right channel
        
        // Update phase
        synthData->osc1.phase += (float)(TWO_PI * freqOsc1 / SAMPLE_RATE);
        if (synthData->osc1.phase >= TWO_PI) synthData->osc1.phase -= TWO_PI;

        synthData->osc2.phase += (float)(TWO_PI * freqOsc2 / SAMPLE_RATE);
        if (synthData->osc2.phase >= TWO_PI) synthData->osc2.phase -= TWO_PI;
    }

    return paContinue;
}

int start_audio(AudioData *data, PaStream **stream) {

    AudioData *audioData = (AudioData*)data;
    SynthData *synthData = &audioData->synthData;
    // ArpData *arpData = &audioData->arpData;

    pthread_mutex_init(&synthData->lock, NULL);
    synthData->lowpass_last_sample = 0.0f;
    update_lowpass_alpha(synthData);
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

