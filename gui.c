#define RAYGUI_IMPLEMENTATION

#include <stdbool.h>
#include <stdio.h>
#include "raygui.h"
#include "raylib.h"
#include "synth.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 250
#define SLIDER_WIDTH_LONG 100
#define SLIDER_WIDTH_SHORT 48
#define SLIDER_HEIGHT 20
#define BUTTON_WIDTH 40
#define BUTTON_HEIGHT 20
#define KEY_WIDTH 30
#define KEY_HEIGHT 40

#define NUM_WHITE_KEYS 7
#define NUM_BLACK_KEYS 5
#define BASE_NOTE 60 // C4 = MIDI

float gui_frequency = 440.0f;
bool whiteKeyStates[NUM_WHITE_KEYS] = { false };
bool blackKeyStates[NUM_BLACK_KEYS] = { false };
bool prevWhiteKeyStates[NUM_WHITE_KEYS] = { false };
bool prevBlackKeyStates[NUM_BLACK_KEYS] = { false };
int whiteOffsets[NUM_WHITE_KEYS] = { 0, 2, 4, 5, 7, 9, 11 }; // Semitones from C4
int blackOffsets[NUM_BLACK_KEYS] = { 1, 3, 6, 8, 10 };
Rectangle whiteKeyRects[NUM_WHITE_KEYS] = {
    (Rectangle){ 71,  200, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){ 109, 200, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){ 147, 200, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){ 185, 200, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){ 223, 200, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){ 261, 200, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){ 299, 200, KEY_WIDTH, KEY_HEIGHT},
};
Rectangle blackKeyRects[NUM_BLACK_KEYS] = {
    (Rectangle){90,  155, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){128, 155, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){204, 155, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){242, 155, KEY_WIDTH, KEY_HEIGHT},
    (Rectangle){280, 155, KEY_WIDTH, KEY_HEIGHT},
};

int run_gui(AudioData *data) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TB1");
    GuiLoadStyle("./external/style_candy.rgs");
    SetTargetFPS(60);

    SynthData *synthData = &data->synthData;
    ArpData *arpData = &data->arpData;
    EnvData *envData = &data->envData;

    float localAmplitude = synthData->amplitude;
    float localOscMix = synthData->oscMix;
    float localSubMix = synthData->subMix;
    float localDetune = synthData->osc2Detune;
    float localLPResonance = synthData->lowpass_resonance;
    float localLFOFreq = synthData->filter_cutoff_lfo.frequency;
    float localLFODepth = synthData->filter_cutoff_lfo.depth;
    float localAttackTime = envData->attackTime;
    float localDecayTime = envData->decayTime;
    float localSustainLevel = envData->sustainLevel;
    float localReleaseTime = envData->releaseTime;
    float localHoldTime = envData->holdTime;
    float localArpTempo = arpData->arp_tempo;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0xfff5e1ff));
        // OSC1 Waveform selector
        if (GuiButton((Rectangle){ 158, 5, BUTTON_WIDTH, BUTTON_HEIGHT }, waveNames[synthData->osc1.waveform])) {
            pthread_mutex_lock(&synthData->lock);
            synthData->osc1.waveform = (synthData->osc1.waveform + 1) % NUM_WAVEFORMS;
            pthread_mutex_unlock(&synthData->lock);
        }
        
        // OSC2 Waveform selector
        if (GuiButton((Rectangle){ 8, 5, BUTTON_WIDTH, BUTTON_HEIGHT }, waveNames[synthData->osc2.waveform])) {
            pthread_mutex_lock(&synthData->lock);
            synthData->osc2.waveform = (synthData->osc2.waveform + 1) % NUM_WAVEFORMS;
            pthread_mutex_unlock(&synthData->lock);
        }

        // Oscillator mix slider
        if (GuiSlider((Rectangle){ 53, 5, SLIDER_WIDTH_LONG, SLIDER_HEIGHT }, NULL, NULL, &localOscMix, 0.0f, 1.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&synthData->lock);
            synthData->oscMix = localOscMix;
            pthread_mutex_unlock(&synthData->lock);
        }
        // Detune slider
        if (GuiSlider((Rectangle){ 105, 29, SLIDER_WIDTH_SHORT, SLIDER_HEIGHT }, NULL, "DETUNE", &localDetune, -12.0f, 12.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&synthData->lock);
            synthData->osc2Detune = localDetune;
            pthread_mutex_unlock(&synthData->lock);
        }

        // Amplitude slider
        if (GuiSlider((Rectangle){ 247, 5, SLIDER_WIDTH_SHORT, 20 }, "VOL", NULL, &localAmplitude, 0.0f, 0.3f)) {
            pthread_mutex_lock(&synthData->lock);
            synthData->amplitude = localAmplitude;
            pthread_mutex_unlock(&synthData->lock);
        }

        // Sub mix slider
        if (GuiSlider((Rectangle){ 53, 29, SLIDER_WIDTH_SHORT, 20 }, "SUB", NULL, &localSubMix, 0.0f, 1.0f)) {
            pthread_mutex_lock(&synthData->lock);
            synthData->subMix = localSubMix;
            pthread_mutex_unlock(&synthData->lock);
        }

        // Highpass filter slider
        // Convert actual cutoff frequency to log scale (range: log(20) to log(20000))
        float logMinHP = log10f(20.0f);
        float logMaxHP = log10f(20000.0f);
        float logCutoffHP = log10f(synthData->highpass_cutoff);
        if (GuiSlider((Rectangle){ 299, 5, SLIDER_WIDTH_SHORT, 20 }, NULL, "HP", &logCutoffHP, logMinHP, logMaxHP)) {
            pthread_mutex_lock(&synthData->lock);
            synthData->highpass_cutoff = powf(10.0f, logCutoffHP);
            update_highpass_alpha(synthData);
            pthread_mutex_unlock(&synthData->lock);
        }

        // Lowpass filter slider
        // Convert actual cutoff frequency to log scale (range: log(20) to log(20000))
        float logMinLP = log10f(20.0f);
        float logMaxLP = log10f(20000.0f);
        float logCutoffLP = log10f(synthData->lowpass_cutoff);
        if (GuiSlider((Rectangle){ 247, 29, SLIDER_WIDTH_LONG, 20 }, "LP", NULL, &logCutoffLP, logMinLP, logMaxLP)) {
            pthread_mutex_lock(&synthData->lock);
            synthData->lowpass_cutoff = powf(10.0f, logCutoffLP);
            update_lowpass_alpha(synthData);
            pthread_mutex_unlock(&synthData->lock);
        }

        // Lowpass resonance slider
        if (GuiSlider((Rectangle){ 247, 53, SLIDER_WIDTH_LONG, 20 }, "RES", NULL, &localLPResonance, 0.0f, 1.5f)) {
            pthread_mutex_lock(&synthData->lock);
            synthData->lowpass_resonance = localLPResonance;
            update_lowpass_alpha(synthData);
            pthread_mutex_unlock(&synthData->lock);
        }

        // Filter LFO Freq Slider
        if (GuiSlider((Rectangle){ 247, 77, SLIDER_WIDTH_LONG, 20 }, "LFO", NULL, &localLFOFreq, 0.0f, 20.0f)) {
            pthread_mutex_lock(&synthData->lock);
            synthData->filter_cutoff_lfo.frequency = localLFOFreq;
            update_lowpass_alpha(synthData);
            pthread_mutex_unlock(&synthData->lock);
        }
        
        // Filter LFO Depth Slider
        if (GuiSlider((Rectangle){ 247, 101, SLIDER_WIDTH_LONG, 20 }, "DEPH", NULL, &localLFODepth, 0.0f, 200.0f)) {
            pthread_mutex_lock(&synthData->lock);
            synthData->filter_cutoff_lfo.depth = localLFODepth;
            update_lowpass_alpha(synthData);
            pthread_mutex_unlock(&synthData->lock);
        }
        
        // Playback mode button
        if (GuiButton((Rectangle){ 16, 198, BUTTON_WIDTH, BUTTON_HEIGHT }, playbackModes[arpData->playback_mode])) {
            pthread_mutex_lock(&arpData->lock);
            arpData->playback_mode = (arpData->playback_mode + 1) % NUM_PLAYBACK_MODES;
            pthread_mutex_unlock(&arpData->lock);
        }

        // Envelope sliders
        // Attack
        if (GuiSlider((Rectangle){ 53, 53, SLIDER_WIDTH_LONG, 20 }, "ATCK", NULL, &localAttackTime, 0.0f, 1.0f)) {
            pthread_mutex_lock(&envData->lock);
            envData->attackTime = localAttackTime;
            pthread_mutex_unlock(&envData->lock);
        }
        // Decay
        if (GuiSlider((Rectangle){ 53, 77, SLIDER_WIDTH_LONG, 20 }, "DECY", NULL, &localDecayTime, 0.0f, 1.0f)) {
            pthread_mutex_lock(&envData->lock);
            envData->decayTime = localDecayTime;
            pthread_mutex_unlock(&envData->lock);
        }
        // Sustain
        if (GuiSlider((Rectangle){ 53, 101, SLIDER_WIDTH_LONG, 20 }, "SUST", NULL, &localSustainLevel, 0.0f, 1.0f)) {
            pthread_mutex_lock(&envData->lock);
            envData->sustainLevel = localSustainLevel;
            pthread_mutex_unlock(&envData->lock);
        }
        // Release
        if (GuiSlider((Rectangle){ 53, 125, SLIDER_WIDTH_LONG, 20 }, "RELE", NULL, &localReleaseTime, 0.0f, 1.0f)) {
            pthread_mutex_lock(&envData->lock);
            envData->releaseTime = localReleaseTime;
            pthread_mutex_unlock(&envData->lock);
        }
        // Hold
        if (GuiSlider((Rectangle){ 247, 125, SLIDER_WIDTH_SHORT, 20 }, "HOLD", NULL, &localHoldTime, 0.0f, 1.0f)) {
            pthread_mutex_lock(&envData->lock);
            envData->holdTime = localHoldTime;
            pthread_mutex_unlock(&envData->lock);
        }
        // Arp tempo
        if (GuiSlider((Rectangle){ 299, 125, SLIDER_WIDTH_SHORT, 20 }, NULL, "TMPO", &localArpTempo, 0.05f, 1.5f)) {
            pthread_mutex_lock(&envData->lock);
            arpData->arp_tempo = localArpTempo;
            pthread_mutex_unlock(&envData->lock);
        }
        // Draw white keys
        for (int i = 0; i < NUM_WHITE_KEYS; i++) {
            GuiToggle(whiteKeyRects[i], NULL, &whiteKeyStates[i]);
            switch (arpData->playback_mode) {
                case SINGLE:
                    if (whiteKeyStates[i]) {
                        memset(whiteKeyStates, 0, sizeof(whiteKeyStates));
                        memset(blackKeyStates, 0, sizeof(blackKeyStates));
                        whiteKeyStates[i] = true;
                        int midiNote = BASE_NOTE + whiteOffsets[i];
                        clear_arp_notes(arpData);
                        add_arp_note(arpData, midiNote);
                        trigger_envelope(envData);
                    }
                    break;

                case ARP:
                    if (whiteKeyStates[i] != prevWhiteKeyStates[i]) {
                        int midiNote = BASE_NOTE + whiteOffsets[i];
                        if (whiteKeyStates[i]) {
                            add_arp_note(arpData, midiNote);
                        } else {
                            remove_arp_note(arpData, midiNote);
                        }
                        prevWhiteKeyStates[i] = whiteKeyStates[i];
                    }
                    break;
            }
        }

        // Draw black keys
        for (int i = 0; i < NUM_BLACK_KEYS; i++) {
            GuiToggle(blackKeyRects[i], NULL, &blackKeyStates[i]);
            switch (arpData->playback_mode) {
                case SINGLE:
                    if (blackKeyStates[i]) {
                        memset(whiteKeyStates, 0, sizeof(whiteKeyStates));
                        memset(blackKeyStates, 0, sizeof(blackKeyStates));
                        blackKeyStates[i] = true;
                        int midiNote = BASE_NOTE + blackOffsets[i];
                        clear_arp_notes(arpData);
                        trigger_envelope(envData);
                        add_arp_note(arpData, midiNote);
                    }
                    break;

                case ARP:
                    if (blackKeyStates[i] != prevBlackKeyStates[i]) {
                        int midiNote = BASE_NOTE + blackOffsets[i];
                        if (blackKeyStates[i]) {
                            add_arp_note(arpData, midiNote);
                        } else {
                            remove_arp_note(arpData, midiNote);
                        }
                        prevBlackKeyStates[i] = blackKeyStates[i];
                    }
                    break;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
