#include "synth.h"

typedef enum { // Can be removed?
    C,
    Db,
    D,
    Eb,
    E,
    F,
    Gb,
    G,
    Ab,
    A,
    Bb,
    B,
} Pitch;

const float *pitchHz[12];

float calculate_frequency(int midiNote, float detune);

void update_arp(SynthData *data);
void add_arp_note(SynthData *data, int midiNote);
void remove_arp_note(SynthData *data, int midiNote);
