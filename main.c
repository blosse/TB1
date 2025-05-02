
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
    SynthData synthData = {
        .osc1 = osc1,
        .osc2 = osc2,
        .osc2Detune = 0.0f,
        .amplitude = 0.25f,
        .oscMix = 0.5f,
        .lowpass_cutoff = 10000.0f,
        .lowpass_resonance = 0.0f,
        .highpass_cutoff = 10000.0f,
        .highpass_prev_input = 0.5f,
        .highpass_prev_output = 0.5f,
    };
    ArpData arpData = {
        .arp_note_count = 0,
        .arp_notes = { -1 },
        .arp_interval = 0.25f,
    };
    AudioData data = {
        .synthData = synthData,
        .arpData = arpData,
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
