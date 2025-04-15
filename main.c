
/* Main file for the TB1, my little synth project */

#include <stdio.h>
#include "gui.h"
#include "synth.h"
#include "waveform.h"

int main() {

    Oscillator osc1 = {
        .phase = 0.0f,
        .frequency = 440.0f,
        .waveform = WAVE_SINE,
    };
    Oscillator osc2 = {
        .phase = 0.0f,
        .frequency = 660.0f,
        .waveform = WAVE_SINE,
    };
    SynthData data = {
        .osc1 = osc1,
        .osc2 = osc2,
        .osc2Detune = 0.0f,
        .oscMix = 0.5f,
        .highpass_cutoff = 20.0f,
        .highpass_prev_input = 0.0f,
        .highpass_prev_output = 0.0f,
    };

    PaStream *stream;
    if (start_audio(&data, &stream) != paNoError) {
        fprintf(stderr, "Failed to start audio.\n");
        return 1;
    }

    run_gui(&data);

    stop_audio(stream);

    return 0;
}
