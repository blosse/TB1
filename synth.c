
/* A little synth project */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <portaudio.h>
#include "waveform.h"

#define SAMPLE_RATE 44100
#define FREQUENCY 440.0
#define AMPLITUDE 0.25
#define TWO_PI (3.14159265 * 2)
#define FRAMES_PER_BUFFER 64

typedef struct {
    float phase;
    WaveformType waveform;
} SynthData;

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

    SynthData *data = (SynthData*)userData;
    float *out = (float*)outputBuffer;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float sample = AMPLITUDE * generate_sample(data->waveform, data->phase);
        *out++ = sample; // left channel
        *out++ = sample; // right channel

        data->phase += (float)(TWO_PI * FREQUENCY / SAMPLE_RATE);
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

int main(int argc, char *argv[]) {

    Pa_Initialize();
    WaveformType selectedWaveform = WAVE_SINE;

    if (argc > 1) {
        selectedWaveform = parse_waveform(argv[1]);
    }

    SynthData data = {
        .phase = 0.0f,
        .waveform = selectedWaveform
    };
    PaStream *stream;

    Pa_OpenDefaultStream(&stream,
                         0,
                         2,
                         paFloat32,
                         SAMPLE_RATE,
                         FRAMES_PER_BUFFER,
                         audioCallback,
                         &data);

    Pa_StartStream(stream);

    printf("Playing a %s. Press Enter to stop\n", waveNames[data.waveform]);
    getchar();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
