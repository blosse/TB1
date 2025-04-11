#define RAYGUI_IMPLEMENTATION

#include <stdbool.h>
#include "raygui.h"
#include "raylib.h"
#include "synth.h"
#include "pitch.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 250
#define SLIDER_LENGTH_LONG (WINDOW_WIDTH - 90 - 60)
#define SLIDER_LENGTH_SHORT (SLIDER_LENGTH_LONG / 2 - 5)
#define KEY_WIDTH 30
#define KEY_HEIGHT 40

#define NUM_WHITE_KEYS 7
#define NUM_BLACK_KEYS 5
#define BASE_NOTE 60 // C4 = MIDI

float gui_frequency = 440.0f;
bool whiteKeyStates[NUM_WHITE_KEYS] = { false };
bool blackKeyStates[NUM_BLACK_KEYS] = { false };
int whiteOffsets[NUM_WHITE_KEYS] = {0, 2, 4, 5, 7, 9, 11}; // Semitones from C4

int run_gui(SynthData *data) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TB1");
    GuiLoadStyle("./external/style_candy.rgs");
    SetTargetFPS(60);

    float localFreq1 = data->osc1.frequency;
    float localFreq2 = data->osc2.frequency;
    float localOscMix = data->oscMix;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0xfff5e1ff));
        
        // OSC1 Waveform selector
        if (GuiButton((Rectangle){ WINDOW_WIDTH - 50, 15, 40, 20 }, waveNames[data->osc1.waveform])) {
            pthread_mutex_lock(&data->lock);
            data->osc1.waveform = (data->osc1.waveform + 1) % NUM_WAVEFORMS;
            pthread_mutex_unlock(&data->lock);
        }

        // OSC1 Frequency slider
        /*if (GuiSlider((Rectangle){ 90, 15, SLIDER_LENGTH_LONG, 20 },TextFormat("OSC1 %.1f Hz", localFreq1), NULL, &localFreq1, 220.0f, 880.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->osc1.frequency = localFreq1;
            pthread_mutex_unlock(&data->lock);
        }*/
        
        // OSC2 Waveform selector
        if (GuiButton((Rectangle){ WINDOW_WIDTH - 50, 55, 40, 20 }, waveNames[data->osc2.waveform])) {
            pthread_mutex_lock(&data->lock);
            data->osc2.waveform = (data->osc2.waveform + 1) % NUM_WAVEFORMS;
            pthread_mutex_unlock(&data->lock);
        }

        // Frequency slider
        /*if (GuiSlider((Rectangle){ 90, 55, SLIDER_LENGTH_LONG, 20 },TextFormat("OSC2 %.1f Hz", localFreq2), NULL, &localFreq2, 220.0f, 880.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->osc2.frequency = localFreq2;
            pthread_mutex_unlock(&data->lock);
        }*/

        // Oscillator mix slider
        if (GuiSlider((Rectangle){ 90, 95, SLIDER_LENGTH_SHORT, 20 },"OSC MIX", NULL, &localOscMix, 0.0f, 1.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->oscMix = localOscMix;
            pthread_mutex_unlock(&data->lock);
        }

        // Lowpass filter slider
        // Convert actual cutoff frequency to log scale (range: log(20) to log(20000))
        float logMin = log10f(20.0f);
        float logMax = log10f(20000.0f);
        float logCutoff = log10f(data->lowpass_cutoff);
        if (GuiSlider((Rectangle){ 100 + SLIDER_LENGTH_SHORT, 95, SLIDER_LENGTH_SHORT, 20 }, NULL, "LP", &logCutoff, logMin, logMax)) {
            pthread_mutex_lock(&data->lock);
            data->lowpass_cutoff = powf(10.0f, logCutoff);
            update_lowpass_alpha(data);
            pthread_mutex_unlock(&data->lock);
        }
        
        // Draw keys 
        for (int i = 0; i < NUM_WHITE_KEYS; i++) {
            Rectangle keyRect = { 80 + (i * (KEY_WIDTH + 5)), 200, KEY_WIDTH, KEY_HEIGHT };

            GuiToggle(keyRect, NULL, &whiteKeyStates[i]);

            if (whiteKeyStates[i]) {
                memset(whiteKeyStates, 0, sizeof(whiteKeyStates));
                whiteKeyStates[i] = true;
                int midiNote = BASE_NOTE + whiteOffsets[i];
                float freq = calculate_frequency(midiNote);
                pthread_mutex_lock(&data->lock);
                data->osc1.frequency = freq;
                data->osc2.frequency = freq;
                pthread_mutex_unlock(&data->lock);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
