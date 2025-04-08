#include "waveform.h"
#include <math.h>

#define TWO_PI 6.28318531f

const char *waveNames[NUM_WAVEFORMS] = {
    "sine",
    "square",
    "triangle",
    "saw" };

float generate_sample(WaveformType type, float phase) {
    switch (type) {
        case WAVE_SINE:
            return sinf(phase);
        case WAVE_SQUARE:
            return (sinf(phase) >= 0.0f) ? 1.0f : -1.0f;
        case WAVE_TRIANGLE:
            return (2.0f / M_PI) * asinf(sinf(phase));
        case WAVE_SAW:
            return (2.0f / M_PI) * (phase - M_PI) / TWO_PI;
        default:
            return 0.0f;
    }
}

