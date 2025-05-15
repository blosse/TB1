#pragma once
#ifndef MAX_ARP_NOTES
#define MAX_ARP_NOTES 8
#endif

#include <pthread.h>
#include <stdbool.h>

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
    int last_note_index;
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
int update_arp(ArpData *data);
void add_arp_note(ArpData *data, int midiNote);
void remove_arp_note(ArpData *data, int midiNote);
void clear_arp_notes(ArpData *data);
int get_arp_note(ArpData *data);

// Envelope stuff
float update_envelope(EnvData *env);
void reset_envelope_stage(EnvData *env);
