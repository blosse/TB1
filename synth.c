
/* A little synth project */

#include <stdio.h>
#include <math.h>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FREQUENCY 440.0
#define AMPLITUDE 0.25
#define TWO_PI (3.14159265 * 2)
#define FRAMES_PER_BUFFER 64

typedef struct {
    float phase;
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
        float sample = AMPLITUDE * sinf(data->phase);
        *out++ = sample; // left channel
        *out++ = sample; // right channel

        data->phase += (float)(TWO_PI * FREQUENCY / SAMPLE_RATE);
        if (data->phase >= TWO_PI) data->phase -= TWO_PI;
    }

    return paContinue;
}

int main() {

    Pa_Initialize();

    SynthData data = { .phase = 0.0f };
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

    printf("Playing a sinewave. Press Enter to stop\n");
    getchar();

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
