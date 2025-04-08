#ifndef WAVEFORM_H
#define WAVEFORM_H
#define NUM_WAVEFORMS 4

typedef enum {
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_TRIANGLE,
    WAVE_SAW
} WaveformType;

float generate_sample(WaveformType type, float phase);

const char *waveNames[NUM_WAVEFORMS];

#endif
