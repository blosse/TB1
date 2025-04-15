#include <math.h>
#include "pitch.h"
#include <pthread.h>

float calculate_frequency(int midiNote, float detune) {
    return 440.0f * powf(2.0f, (midiNote - 69 + detune) / 12.0f);  // A4 = MIDI 69 = 440Hz
}

void update_arp(SynthData *data) {
    if (data->arp_note_count == 0) return;

    data->arp_time += 1.0f / SAMPLE_RATE;
    if (data->arp_time >= data->arp_interval) {
        data->arp_time = 0.0f;

        data->arp_index = (data->arp_index + 1) % data->arp_note_count;
        int midiNote = data->arp_notes[data->arp_index];

        float freq1 = calculate_frequency(midiNote, 0.0f);
        float freq2 = calculate_frequency(midiNote, data->osc2Detune);

        pthread_mutex_lock(&data->lock);
        data->osc1.frequency = freq1;
        data->osc2.frequency = freq2;
        pthread_mutex_unlock(&data->lock);
    }
}

void add_arp_note(SynthData *data, int midiNote) {
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

void remove_arp_note(SynthData *data, int midiNote) {
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
