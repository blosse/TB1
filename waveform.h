#ifndef WAVEFORM_H
#define WAVEFORM_H
#define NUM_WAVEFORMS 4

typedef enum {
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_SAW
} WaveformType;

const char *waveNames[NUM_WAVEFORMS];

float generate_sample(WaveformType type, float phase);

#endif
