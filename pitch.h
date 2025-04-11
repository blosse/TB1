
typedef enum {
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

