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
    
    SynthData *data = (SynthData*)userData;
    float freq;
    pthread_mutex_lock(&data->lock);
    freq = data->frequency;
    pthread_mutex_unlock(&data->lock);

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float sample = AMPLITUDE * generate_sample(data->waveform, data->phase);
        *out++ = sample; // left channel
        *out++ = sample; // right channel

        data->phase += (float)(TWO_PI * freq / SAMPLE_RATE);
        if (data->phase >= TWO_PI) data->phase -= TWO_PI;
    }

    return paContinue;
}

WaveformType parse_waveform(const char *arg) {
    if (strcmp(arg, "sine") == 0) return WAVE_SINE;
    if (strcmp(arg, "square") == 0) return WAVE_SQUARE;
    if (strcmp(arg, "triangle") == 0) return WAVE_TRIANGLE;
    if (strcmp(arg, "saw") == 0) return WAVE_SAW;

    // Default
    printf("Whoops, incorrect wave, playing a sine");
    return WAVE_SINE;
}

int start_audio(SynthData *data, PaStream **stream) {
    data->phase = 0.0f;
    data->frequency = 440.0f;
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

