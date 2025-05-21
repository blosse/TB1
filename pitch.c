#include <math.h>
#include <stdio.h>
#include "pitch.h"
#include "synth.h"
#include <pthread.h>

const char *playbackModes[NUM_PLAYBACK_MODES] = {
    "NOTE",
    "ARP",
};

float calculate_frequency(int midiNote, float detune) {
  if (midiNote <= 0) {
    return 0.0f;
  };
  return 440.0f *
         powf(2.0f, (midiNote - 69 + detune) / 12.0f); // A4 = MIDI 69 = 440Hz
}

// Updates the arp index and returns the current note to be played
// This function should maybe trigger envelope?
void update_arp(ArpData *data) {
    if (data->arp_note_count == 0) return;

    data->arp_time += 1.0f / (SAMPLE_RATE * data->arp_tempo);
    if (data->arp_time >= data->arp_interval) {
        data->arp_time = 0.0f;
        data->arp_index = (data->arp_index + 1) % data->arp_note_count;

        // Fire callback when new note is played
        // Callback updates oscillator freqs
        // and triggers envelope
        if (data->callback) {
            int note = data->arp_notes[data->arp_index];
            data->callback(note, data->callbackUserData);
        }
    }
    return;
}

int get_arp_note(ArpData *data) {
    return data->arp_notes[data->arp_index];
}

void arp_set_callback(ArpData *data, ArpNoteCallback cb, void *userData) {
    data->callback = cb;
    data->callbackUserData = userData;
}

void add_arp_note(ArpData *data, int midiNote) {
    // This thing is a mess
    // Big 'ol if ladder here, I don't love it
    printf("Add note: %d\n", midiNote);
    if (data->arp_note_count >= MAX_ARP_NOTES) {
        printf("Max number of arp notes\n");
        return;
    }
    pthread_mutex_lock(&data->lock);
    if (data->arp_note_count < 1) {
        data->arp_notes[data->arp_note_count++] = midiNote;
        pthread_mutex_unlock(&data->lock);
        return;
    }
    if (data->arp_note_count == 1) {
        if (midiNote > data->arp_notes[0]) {
            data->arp_notes[data->arp_note_count++] = midiNote;
        }
        else {
            data->arp_notes[1] = data->arp_notes[0];
            data->arp_notes[0] = midiNote;
            data->arp_note_count++;
        }
        pthread_mutex_unlock(&data->lock);
        return;
    }
    if (midiNote < data->arp_notes[0]) {
        data->arp_note_count++;
        int j = data->arp_note_count-1;
        for (; j >= 0; j--) {
            data->arp_notes[j+1] = data->arp_notes[j];
        }
        data->arp_notes[0] = midiNote;
        pthread_mutex_unlock(&data->lock);
        return;
    }
    for (int i = 0; i < data->arp_note_count; i++) {
        if (data->arp_notes[i] == midiNote) {
            pthread_mutex_unlock(&data->lock);
            return;
        }
        if ((data->arp_notes[i] < midiNote && data->arp_notes[i+1] > midiNote)) {
            // Nested for loops yeehaw
            // Also some duplicate code here...
            data->arp_note_count++;
            for (int j  = data->arp_note_count-1; j > i; j--) {
                data->arp_notes[j+1] = data->arp_notes[j];
            }
            data->arp_notes[i+1] = midiNote;
            pthread_mutex_unlock(&data->lock);
        }
    }
    data->arp_notes[data->arp_note_count++] = midiNote;
    pthread_mutex_unlock(&data->lock);
    return;
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

float update_envelope(EnvData *env) {
    float dt = 1.0f / SAMPLE_RATE;
    switch (env->stage) {
        case 1: // Attack
            env->currentValue += dt / env->attackTime;
            if (env->currentValue >= 1.0f) {
                env->currentValue = 1.0f;
                env->stage = 2;
            }
            break;
        case 2: // Decay
            env->currentValue -= dt / env->decayTime * (1.0f - env->sustainLevel);
            if (env->currentValue <= env->sustainLevel) {
                env->currentValue = env->sustainLevel;
                env->stage = 3;
            }
            break;
        case 3: // Sustain
            env->currentHoldValue += dt;
            // TODO: Why has arp mode broke?
            if (env->currentHoldValue >= env->holdTime) {
                env->currentHoldValue = 0.0f;
                env->stage = 4;
            }
            break;
        case 4: // Release
            env->currentValue -= dt / env->releaseTime;
            if (env->currentValue <= 0.0f) {
                env->currentValue = 0.0f;
                env->stage = 0; // Idle
            }
            break;
    }
    return env->currentValue;
}

void trigger_envelope(EnvData *env) {
    pthread_mutex_lock(&env->lock);
    env->stage = 1;
    env->currentHoldValue = 0.0f;
    pthread_mutex_unlock(&env->lock);
}
