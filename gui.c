#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib.h"
#include "synth.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 250

float gui_frequency = 440.0f;

int run_gui(SynthData *data) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TB1");
    SetTargetFPS(60);

    float localFreq = data->frequency;
    int currentWave = data->waveform;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        //Waveform selector
        int previousWave = data->waveform;
        GuiToggleGroup((Rectangle){ 20, 140, 40, 30 }, "SINE;SQR;TRI;SAW", &currentWave);
        if (previousWave != currentWave) {
            pthread_mutex_lock(&data->lock);
            data->waveform = (WaveformType) currentWave;
            pthread_mutex_unlock(&data->lock);
        }

        // Frequency slider
        if (GuiSlider((Rectangle){ 90, 80, WINDOW_WIDTH - 90 - 20, 20 },TextFormat("OSC1 %.1f Hz", localFreq), NULL, &localFreq, 220.0f, 880.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->frequency = localFreq;
            pthread_mutex_unlock(&data->lock);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
