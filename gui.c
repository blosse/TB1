#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib.h"
#include "synth.h"

float gui_frequency = 440.0f;

int run_gui(SynthData *data) {
    InitWindow(400, 250, "TB1");
    SetTargetFPS(60);

    float localFreq = data->frequency;
    int currentWave = data->waveform;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        //Waveform selector
        const char *labels[] = { "SINE", "SQUARE", "TRIANGLE", "SAW" };
        for (int i = 0; i < 4; i++) {
            Rectangle rect = { 20 + i * 90, 140, 80, 30 };
            bool active = (currentWave == i);
            bool pressed = GuiToggle(rect, labels[i], & active );
            if (pressed && active) {
                currentWave = i;
                pthread_mutex_lock(&data->lock);
                data->waveform = (WaveformType) i;
                pthread_mutex_unlock(&data->lock);
            }
        }

        // Frequency slider
        if (GuiSliderBar((Rectangle){ 40, 80, 340, 20 }, "Freq", NULL, &localFreq, 220.0f, 880.0f)) {
            // If user moved slider, update shared synth frequency
            pthread_mutex_lock(&data->lock);
            data->frequency = localFreq;
            pthread_mutex_unlock(&data->lock);
        }
        DrawText(TextFormat("Frequency: %.1f Hz", localFreq), 20, 120, 20, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
