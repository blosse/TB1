#ifndef MAX_ARP_NOTES
#define MAX_ARP_NOTES 8
#endif

#ifndef SYNTH_H
#include <pthread.h>
#endif

#define SAMPLE_RATE 44100
#define NUM_PLAYBACK_MODES 2

typedef struct {
    int arp_notes[MAX_ARP_NOTES];
    int arp_index;
    int arp_note_count;
    float arp_time;
    float arp_interval;
    int arp_mode;
    int playback_mode;

    pthread_mutex_t lock;
} ArpData;

enum playback_mode {
    SINGLE,
    ARP,
};

enum arp_mode {
    DOWN,
    UP,
    UP_DOWN,
    RANDOM,
};

const char *playbackModes[NUM_PLAYBACK_MODES];

float calculate_frequency(int midiNote, float detune);

void update_arp(ArpData *data);
void add_arp_note(ArpData *data, int midiNote);
void remove_arp_note(ArpData *data, int midiNote);
void clear_arp_notes(ArpData *data);
int get_arp_note(ArpData *data);
