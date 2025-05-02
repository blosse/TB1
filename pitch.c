#include <math.h>
#include <stdio.h>
#include "pitch.h"
#include <pthread.h>

const char *playbackModes[NUM_PLAYBACK_MODES] = {
    "NOTE",
    "ARP",
};

float calculate_frequency(int midiNote, float detune) {
    return 440.0f * powf(2.0f, (midiNote - 69 + detune) / 12.0f);  // A4 = MIDI 69 = 440Hz
}

void update_arp(ArpData *data) {
    if (data->arp_note_count == 0) {
        // printf("Inside update_arp - no notes\n");
        return;
    }

    data->arp_time += 1.0f / SAMPLE_RATE;
    if (data->arp_time >= data->arp_interval) {
        data->arp_time = 0.0f;

        data->arp_index = (data->arp_index + 1) % data->arp_note_count;
    }
}

int get_arp_note(ArpData *data) {
    return data->arp_notes[data->arp_index];
}

void add_arp_note(ArpData *data, int midiNote) {
    pthread_mutex_lock(&data->lock);
    for (int i = 0; i < data->arp_note_count; i++) {
        if (data->arp_notes[i] == midiNote) {
            pthread_mutex_unlock(&data->lock);
            return;
        }
    }
    if (data->arp_note_count < MAX_ARP_NOTES) {
        data->arp_notes[data->arp_note_count++] = midiNote;
    }
    pthread_mutex_unlock(&data->lock);
}

void remove_arp_note(ArpData *data, int midiNote) {
    pthread_mutex_lock(&data->lock);
    for (int i = 0; i < data->arp_note_count; i++) {
        if (midiNote == data->arp_notes[i]) {
            for (int j = i; j < data->arp_note_count - 1; j++) {
                data->arp_notes[j] = data->arp_notes[j+1];
            }
            data->arp_note_count--;
            break;
        }
    }
    pthread_mutex_unlock(&data->lock);
}

void clear_arp_notes(ArpData *data) {
    pthread_mutex_lock(&data->lock);
    data->arp_note_count = 0;
    pthread_mutex_unlock(&data->lock);
}
