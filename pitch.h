#pragma once

#include <pthread.h>
#include <stdbool.h>

#define SAMPLE_RATE 44100
#define NUM_PLAYBACK_MODES 2
#define MAX_ARP_NOTES 8
#define NUM_ARP_MODES 5

typedef void (*ArpNoteCallback)(int midiNote, void *userData);

typedef struct {
    int arp_notes[MAX_ARP_NOTES];
    int arp_index;
    int arp_note_count;
    float arp_time;
    float arp_tempo;
    float arp_interval;
    int arp_mode;
    int up_down_dir;
    int playback_mode;
    int last_note_index;
    pthread_mutex_t lock;

    ArpNoteCallback callback;
    void *callbackUserData;
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
    ROBYN,
};

const char *playbackModes[NUM_PLAYBACK_MODES];
const char *arpModes[NUM_ARP_MODES];

typedef struct {
    float attackTime;
    float decayTime;
    float sustainLevel;
    float releaseTime;
    float holdTime;
    float currentHoldValue;
    float currentValue;
    float elapsedTime;
    int stage; // 0 = idle, 1 = attack, 2 = decay, 3 = sustain, 4 = release
    pthread_mutex_t lock;
} EnvData;

float calculate_frequency(int midiNote, float detune);

// Arp stuff
void update_arp(ArpData *data);
void add_arp_note(ArpData *data, int midiNote);
void remove_arp_note(ArpData *data, int midiNote);
void clear_arp_notes(ArpData *data);
int get_arp_note(ArpData *data);
void arp_set_callback(ArpData *data, ArpNoteCallback cb, void *userData);

// Envelope stuff
float update_envelope(EnvData *env);
void trigger_envelope(EnvData *env);
