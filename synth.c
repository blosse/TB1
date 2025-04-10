/* A little synth project */


#include "synth.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

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
    float freqOsc1, freqOsc2;
    
    SynthData *data = (SynthData*)userData;

    // Lock the mutex to safely access shared data
    pthread_mutex_lock(&data->lock);
    freqOsc1 = data->osc1.frequency;  // Frequency of the first oscillator
    freqOsc2 = data->osc2.frequency;  // Frequency of the second oscillator
    pthread_mutex_unlock(&data->lock);

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float sampleOsc1 = AMPLITUDE * generate_sample(data->osc1.waveform, data->osc1.phase);
        float sampleOsc2 = AMPLITUDE * generate_sample(data->osc2.waveform, data->osc2.phase);
        float oscMix = AMPLITUDE * ((data->oscMix * sampleOsc1) + ((1 - data->oscMix) * sampleOsc2));
        *out++ = oscMix; // left channel
        *out++ = oscMix; // right channel

        data->osc1.phase += (float)(TWO_PI * freqOsc1 / SAMPLE_RATE);
        if (data->osc1.phase >= TWO_PI) data->osc1.phase -= TWO_PI;

        data->osc2.phase += (float)(TWO_PI * freqOsc2 / SAMPLE_RATE);
        if (data->osc2.phase >= TWO_PI) data->osc2.phase -= TWO_PI;
    }

    return paContinue;
}

int start_audio(SynthData *data, PaStream **stream) {
    pthread_mutex_init(&data->lock, NULL);
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

