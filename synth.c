/* A little synth project */

#include "synth.h"
#include "pitch.h"
#include "waveform.h"
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
    printf("filter cutoff: %f\n", data->lowpass_cutoff_modulated);
    return data->lowpass_stage2;
}

void update_lowpass_alpha(SynthData *data) {
    float rc = 1.0f / (2.0f * M_PI * data->lowpass_cutoff_modulated);
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

float update_filter_cutoff_lfo(LFO *data) {
    if (data->depth <= 0.01f) return 0;

    float value = 0.0f;
    float phase = data->phase;
    
    switch (data->waveform) {
        case WAVE_SINE:
            value = sinf(2 * M_PI * phase);
            break;
        case WAVE_SQUARE:
            value = (phase < 0.5f) ? 1.0f : -1.0f;
            break;
        case WAVE_TRIANGLE:
            value = 4.0f * fabsf(phase - 0.5f) - 1.0f;
            break;
        case WAVE_SAW:
            value = 2.0f * (phase - 0.5f);
            break;
    }

    data->phase += data->frequency / SAMPLE_RATE;
    if ( data->phase >= 1.0f ) data->phase -= 1.0f;

    printf("LFO mod: %f - ", value * data->depth);
    return value * data->depth;
}

void on_new_arp_note(int midiNote, void *data) {
    AudioData *audioData = (AudioData*)data;
    SynthData *synthData = &audioData->synthData;
    EnvData *envData = &audioData->envData;
    update_oscillator_frequencies(synthData, midiNote);
    trigger_envelope(envData);
}

void update_oscillator_frequencies(SynthData *synthData, int midiNote) {
    if (midiNote <= 0) {
        return;
    };
    float freqOsc1 = 440.0f * powf(2.0f, (midiNote - 69) / 12.0f); // A4 = MIDI 69 = 440Hz
    float freqOsc2 = 440.0f * powf(2.0f, (midiNote - 69 + synthData->osc2Detune) / 12.0f);
    float freqOscSub = 440.0f * powf(2.0f, (midiNote - 69 - 24) / 12.0f); // Sub is 2 octaves below osc1

    pthread_mutex_lock(&synthData->lock);
    synthData->osc1.frequency = freqOsc1; // Frequency of the fst oscillator
    synthData->osc2.frequency = freqOsc2; // Frequency of the snd oscillator
    synthData->oscSub.frequency = freqOscSub; // Frequency of the sub oscillator
    pthread_mutex_unlock(&synthData->lock);
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
    float wave1, wave2, waveSub;
    float sampleOsc1, sampleOsc2, sampleOscSub ;
    float mix, mixSub, mixFiltered;
    float env;

    AudioData *audioData = (AudioData*)userData;
    SynthData *synthData = &audioData->synthData;
    ArpData *arpData = &audioData->arpData;
    EnvData *envData = &audioData->envData;
    // Lock the mutex to safely access shared data
    pthread_mutex_lock(&synthData->lock);
    amplitude = synthData->amplitude;
    wave1 = synthData->osc1.waveform;
    wave2 = synthData->osc2.waveform;
    waveSub = synthData->oscSub.waveform;
    mix = synthData->oscMix;
    mixSub = synthData->subMix;
    pthread_mutex_unlock(&synthData->lock);

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        // Updating the frequency each frame now, maybe not necessary
        // Can be changed in future if performance is an issue
        update_arp(arpData);
        env = update_envelope(envData);

        sampleOsc1 = amplitude * env * generate_sample(wave1, synthData->osc1.phase);
        sampleOsc2 = amplitude * env * generate_sample(wave2, synthData->osc2.phase);
        sampleOscSub = amplitude * env * generate_sample(waveSub, synthData->oscSub.phase);

        // Mix oscillators
        float mixedSample = (mix * sampleOsc1) + ((1.0f - mix) * sampleOsc2) + (mixSub * sampleOscSub);

        // Modulate LFO
        synthData->lowpass_cutoff_modulated = synthData->lowpass_cutoff + update_filter_cutoff_lfo(&synthData->filter_cutoff_lfo);
        update_lowpass_alpha(synthData);

        // Apply filters
        float lpFiltered = lowpass_two_stage(mixedSample, synthData);
        mixFiltered = highpass_filter(synthData, lpFiltered);

        *out++ = mixFiltered; // left channel
        *out++ = mixFiltered; // right channel

        // Update phase
        synthData->osc1.phase += (float)(TWO_PI * synthData->osc1.frequency / SAMPLE_RATE);
        if (synthData->osc1.phase >= TWO_PI) synthData->osc1.phase -= TWO_PI;

        synthData->osc2.phase += (float)(TWO_PI * synthData->osc2.frequency / SAMPLE_RATE);
        if (synthData->osc2.phase >= TWO_PI) synthData->osc2.phase -= TWO_PI;

        synthData->oscSub.phase += (float)(TWO_PI * synthData->oscSub.frequency / SAMPLE_RATE);
        if (synthData->oscSub.phase >= TWO_PI) synthData->oscSub.phase -= TWO_PI;
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

