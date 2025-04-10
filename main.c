
/* Main file for the TB1, my little synth project */

#include <stdio.h>
#include "gui.h"
#include "synth.h"
#include "waveform.h"

int main() {

    SynthData data = {
        .waveform = WAVE_SINE,
        .frequency = 440.0f
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
