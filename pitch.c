#include <math.h>
#include "pitch.h"

float calculate_frequency(int midiNote, float detune) {
    return 440.0f * powf(2.0f, (midiNote - 69 + detune) / 12.0f);  // A4 = MIDI 69 = 440Hz
}
