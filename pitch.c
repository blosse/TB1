#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "pitch.h"
#include "synth.h"
#include <pthread.h>

const char *playbackModes[NUM_PLAYBACK_MODES] = {
    "NOTE",
    "ARP",
};
const char *arpModes[NUM_ARP_MODES] = {
    "DOWN",
    "UP",
    "U/D",
    "RNDM",
    "ROBYN",
};
float calculate_frequency(int midiNote, float detune) {
  if (midiNote <= 0) {
    return 0.0f;
  };
  return 440.0f *
         powf(2.0f, (midiNote - 69 + detune) / 12.0f); // A4 = MIDI 69 = 440Hz
}

void update_arp(ArpData *data) {
    if (data->arp_note_count == 0) return;
    int nextNote;
    data->arp_time += 1.0f / (SAMPLE_RATE * data->arp_tempo);
    if (data->arp_time >= data->arp_interval) {
        data->arp_time = 0.0f;
        switch (data->arp_mode) {
            case DOWN:
                data->arp_index = (data->arp_index + 1) % data->arp_note_count;
                break;
            case UP:
                if (data->arp_index > 0)
                    data->arp_index--;
                else
                    data->arp_index = data->arp_note_count - 1;
                break;
            case UP_DOWN:
                if (data->arp_index == 0) {
                    data->up_down_dir = 1;
                } else if (data->arp_index == data->arp_note_count-1) {
                    data->up_down_dir = -1;
                }
                data->arp_index += data->up_down_dir;
                break;
            case RANDOM:
                nextNote = rand() % data->arp_note_count;
                while (data->arp_index == nextNote) {
                    nextNote = rand() % data->arp_note_count;
                }
                    data->arp_index = nextNote;
                break;
            case ROBYN:
                data->arp_index = 0;
                break;
        }

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
    printf("Add note: %d\n", midiNote);
    pthread_mutex_lock(&data->lock);

    if (data->arp_mode == ROBYN) {
        data->arp_note_count = 1;
        data->arp_notes[0] = midiNote;
    } else {
        if (data->arp_note_count >= MAX_ARP_NOTES) {
            printf("Max number of arp notes\n");
            pthread_mutex_unlock(&data->lock);
            return;
        }
        for (int i = 0; i < data->arp_note_count; i++) {
            if (data->arp_notes[i] == midiNote) {
                pthread_mutex_unlock(&data->lock);
                return;
            }
        }
        int insertIndex = 0;
        while(insertIndex < data->arp_note_count && data->arp_notes[insertIndex] < midiNote) {
            insertIndex++;
        }
        for (int i = data->arp_note_count; i > insertIndex; i--) {
            data->arp_notes[i] = data->arp_notes[i-1];
        }
        data->arp_notes[insertIndex] = midiNote;
        data->arp_note_count++;
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
    printf("Envelope triggered\n");
    pthread_mutex_lock(&env->lock);
    env->stage = 1;
    env->currentHoldValue = 0.0f;
    pthread_mutex_unlock(&env->lock);
}
