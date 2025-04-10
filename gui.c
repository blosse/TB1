#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib.h"
#include "synth.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 250
#define SLIDER_LENGTH_LONG (WINDOW_WIDTH - 90 - 60)
#define SLIDER_LENGTH_SHORT (SLIDER_LENGTH_LONG / 2)

float gui_frequency = 440.0f;

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
        if (GuiSlider((Rectangle){ 90, 15, SLIDER_LENGTH_LONG, 20 },TextFormat("OSC1 %.1f Hz", localFreq1), NULL, &localFreq1, 220.0f, 880.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->osc1.frequency = localFreq1;
            pthread_mutex_unlock(&data->lock);
        }
        
        // OSC2 Waveform selector
        if (GuiButton((Rectangle){ WINDOW_WIDTH - 50, 55, 40, 20 }, waveNames[data->osc2.waveform])) {
            pthread_mutex_lock(&data->lock);
            data->osc2.waveform = (data->osc2.waveform + 1) % NUM_WAVEFORMS;
            pthread_mutex_unlock(&data->lock);
        }

        // Frequency slider
        if (GuiSlider((Rectangle){ 90, 55, SLIDER_LENGTH_LONG, 20 },TextFormat("OSC2 %.1f Hz", localFreq2), NULL, &localFreq2, 220.0f, 880.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->osc2.frequency = localFreq2;
            pthread_mutex_unlock(&data->lock);
        }

        // Oscillator mix slider
        if (GuiSlider((Rectangle){ 90, 95, SLIDER_LENGTH_SHORT, 20 },"OSC MIX", NULL, &localOscMix, 0.0f, 1.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->oscMix = localOscMix;
            pthread_mutex_unlock(&data->lock);
        }
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
