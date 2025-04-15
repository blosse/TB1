#define RAYGUI_IMPLEMENTATION

#include <stdbool.h>
#include "raygui.h"
#include "raylib.h"
#include "synth.h"
#include "pitch.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 250
#define SLIDER_WIDTH_LONG (WINDOW_WIDTH / 2)
#define SLIDER_WIDTH_SHORT 100
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

int run_gui(SynthData *data) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TB1");
    GuiLoadStyle("./external/style_candy.rgs");
    SetTargetFPS(60);

    whiteKeyStates[0] = true;
    float localOscMix = data->oscMix;
    float localDetune = data->osc2Detune;
    float localLPResonance = data->lowpass_resonance;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0xfff5e1ff));
        
        // OSC1 Waveform selector
        if (GuiButton((Rectangle){ 5, 5, BUTTON_WIDTH, BUTTON_HEIGHT }, waveNames[data->osc1.waveform])) {
            pthread_mutex_lock(&data->lock);
            data->osc1.waveform = (data->osc1.waveform + 1) % NUM_WAVEFORMS;
            pthread_mutex_unlock(&data->lock);
        }
        
        // OSC2 Waveform selector
        if (GuiButton((Rectangle){ 5, 29, BUTTON_WIDTH, BUTTON_HEIGHT }, waveNames[data->osc2.waveform])) {
            pthread_mutex_lock(&data->lock);
            data->osc2.waveform = (data->osc2.waveform + 1) % NUM_WAVEFORMS;
            pthread_mutex_unlock(&data->lock);
        }

        // Oscillator mix slider
        if (GuiSlider((Rectangle){ 53, 5, SLIDER_WIDTH_SHORT, SLIDER_HEIGHT }, NULL, "OSC MIX", &localOscMix, 0.0f, 1.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->oscMix = localOscMix;
            pthread_mutex_unlock(&data->lock);
        }
        // Detune slider
        if (GuiSlider((Rectangle){ 53, 29, SLIDER_WIDTH_SHORT, SLIDER_HEIGHT }, NULL, "DETUNE", &localDetune, -12.0f, 12.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->osc2Detune = localDetune;
            pthread_mutex_unlock(&data->lock);
        }

        // Highpass filter slider
        // Convert actual cutoff frequency to log scale (range: log(20) to log(20000))
        float logMinHP = log10f(20.0f);
        float logMaxHP = log10f(20000.0f);
        float logCutoffHP = log10f(data->highpass_cutoff);
        if (GuiSlider((Rectangle){ 295, 5, SLIDER_WIDTH_SHORT, 20 }, "HP", NULL, &logCutoffHP, logMinHP, logMaxHP)) {
            pthread_mutex_lock(&data->lock);
            data->highpass_cutoff = powf(10.0f, logCutoffHP);
            update_highpass_alpha(data);
            pthread_mutex_unlock(&data->lock);
        }

        // Lowpass filter slider
        // Convert actual cutoff frequency to log scale (range: log(20) to log(20000))
        float logMinLP = log10f(20.0f);
        float logMaxLP = log10f(20000.0f);
        float logCutoffLP = log10f(data->lowpass_cutoff);
        if (GuiSlider((Rectangle){ 295, 29, SLIDER_WIDTH_SHORT, 20 }, "LP", NULL, &logCutoffLP, logMinLP, logMaxLP)) {
            pthread_mutex_lock(&data->lock);
            data->lowpass_cutoff = powf(10.0f, logCutoffLP);
            update_lowpass_alpha(data);
            pthread_mutex_unlock(&data->lock);
        }

        // Lowpass resonance slider
        if (GuiSlider((Rectangle){ 295, 53, SLIDER_WIDTH_SHORT, 20 }, "RES", NULL, &localLPResonance, 0.0f, 1.0f)) {
            pthread_mutex_lock(&data->lock);
            data->lowpass_resonance = localLPResonance;
            update_lowpass_alpha(data);
            pthread_mutex_unlock(&data->lock);
        }

        // Draw white keys 
        for (int i = 0; i < NUM_WHITE_KEYS; i++) {
            GuiToggle(whiteKeyRects[i], NULL, &whiteKeyStates[i]);
            if (whiteKeyStates[i]) {
                memset(whiteKeyStates, 0, sizeof(whiteKeyStates));
                memset(blackKeyStates, 0, sizeof(blackKeyStates));
                whiteKeyStates[i] = true;
                int midiNote = BASE_NOTE + whiteOffsets[i];
                float freq1 = calculate_frequency(midiNote, 0.0f);
                float freq2 = calculate_frequency(midiNote, data->osc2Detune);
                pthread_mutex_lock(&data->lock);
                data->osc1.frequency = freq1;
                data->osc2.frequency = freq2;
                pthread_mutex_unlock(&data->lock);
            }
        }

        // Draw black keys
        for (int i = 0; i < NUM_BLACK_KEYS; i++) {
            //int whiteKeyIndex = i + (i >= 2 ? 1 : 0);
            GuiToggle(blackKeyRects[i], NULL, &blackKeyStates[i]);
            
            if (blackKeyStates[i]) {
                memset(blackKeyStates, 0, sizeof(blackKeyStates));
                memset(whiteKeyStates, 0, sizeof(whiteKeyStates));
                blackKeyStates[i] = true;
                // Calculate frequency for the toggled black key
                int midiNote = BASE_NOTE + blackOffsets[i];
                float freq1 = calculate_frequency(midiNote, 0.0f);
                float freq2 = calculate_frequency(midiNote, data->osc2Detune);
                pthread_mutex_lock(&data->lock);
                data->osc1.frequency = freq1;
                data->osc2.frequency = freq2;
                pthread_mutex_unlock(&data->lock);
            }


        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
