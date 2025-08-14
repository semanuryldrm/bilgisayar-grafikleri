#include <raylib.h>
#include "kissfft/kiss_fft.h"
#include <stdio.h>
#include <stdlib.h>
#include "fft_queue.h"

#define FFT_SIZE 1024
#define MAX_SONGS 10

kiss_fft_cpx in[FFT_SIZE];
kiss_fft_cpx out[FFT_SIZE];
fft_queue *queue;

float magnitudes[FFT_SIZE / 2];

typedef enum {
    STATE_MENU,
    STATE_VISUALIZER
} AppState;

AppState appState = STATE_MENU;

const char *playlist[MAX_SONGS] = {
    "./1.mp3",
    "./2.mp3",
    "./3.mp3",
    "./5.mp3"
};

int playlistCount = 4;
int selectedSong = 0;
int currentSong = -1;

Sound music = {0};

void run_fft(fft_queue *queue) {
    for (size_t i = 0; i < FFT_SIZE; i++)
    {
        in[i].r = queue->samples[i];
        in[i].i = 0;
    }

    kiss_fft_cfg cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);
    kiss_fft(cfg, in, out);

    for (size_t i = 0; i < FFT_SIZE / 2; i++)
    {
        float re = out[i].r;
        float im = out[i].i;

        magnitudes[i] = sqrtf(re * re + im * im);
    }
    

    free(cfg);
}

void audioCallback(void *buffer, unsigned int frames) {
    float *samples = (float *) buffer;
    unsigned int sampleCount = frames * 2;

    for (size_t i = 0; i < frames; i++)
    {
        float mono = (samples[2 * i] + samples[2 * i + 1]) / 2.0f;
        push_fft_queue(queue, mono);
        if (queue->index >= FFT_SIZE)
        {
            run_fft(queue);
            clear_fft_queue(queue);
        }
        
    }
    
    //printf("frames_count: %d\n", frames);
}

void loadAndPlaySong(const char *filename) {
    if (music.frameCount > 0) {
        DetachAudioStreamProcessor(music.stream, audioCallback);
        UnloadSound(music);
    }

    music = LoadSound(filename);
    if (music.frameCount <= 0) {
        TraceLog(LOG_ERROR, "Failed to load sound: %s", filename);
        return;
    }

    AttachAudioStreamProcessor(music.stream, audioCallback);
    PlaySound(music);
    currentSong = selectedSong;
    clear_fft_queue(queue);
}

void goToNextSong() {
    selectedSong = (currentSong + 1) % playlistCount;
    loadAndPlaySong(playlist[selectedSong]);
}

void goToPrevSong() {
    selectedSong = (currentSong - 1 + playlistCount) % playlistCount;
    loadAndPlaySong(playlist[selectedSong]);
}

int main(int argc, char **argv) {

    queue = malloc(sizeof(fft_queue));
    queue->index = 0;

    InitWindow(800, 600, "music visualizer");
    InitAudioDevice();

    SetExitKey(KEY_NULL);

    music.frameCount = 0;
    
    //Color bg = ColorFromHSV(110, 0.8, 0.7);
    //Color fg = ColorFromHSV(290, 0.8, 0.7);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (appState == STATE_MENU) {

            DrawText("Müzik Seçin:", 20, 20, 30, RAYWHITE);

            for (int i = 0; i < playlistCount; i++) {
                Color color = (i == selectedSong) ? GREEN : GRAY;
                DrawText(TextFormat("%s %s", (i == selectedSong) ? ">" : " ", playlist[i]),
                         40, 80 + i * 30, 20, color);
            }

            DrawText("[↑ ↓] seç - [ENTER] baslat", 20, 500, 20, DARKGRAY);

            if (IsKeyPressed(KEY_DOWN)) selectedSong = (selectedSong + 1) % playlistCount;
            if (IsKeyPressed(KEY_UP)) selectedSong = (selectedSong - 1 + playlistCount) % playlistCount;

            if (IsKeyPressed(KEY_ENTER)) {
                loadAndPlaySong(playlist[selectedSong]);
                appState = STATE_VISUALIZER;
            }
        }

        else if (appState == STATE_VISUALIZER) {
            
            DrawText(TextFormat("Çalan: %s", playlist[currentSong]), 20, 20, 25, RAYWHITE);

            int barWidth = 3;
            int spacing = 1; 
            int totalBars =(FFT_SIZE / 2) / (barWidth + spacing);

            for (size_t i = 0; i < totalBars; i++)
            {
                int index = i * (barWidth + spacing);
                float mag = magnitudes[index];
                if (mag < 1e-6f)
                {
                    mag = 1e-6f;
                }
                
                int height = (int)(200.0f * log10f(mag));
                if (height < 0)
                {
                    height = 0;
                }
                int x = i *(barWidth + spacing);
                int y = 600 - height;

                float hue = (360.0f * i) / (FFT_SIZE / 2);
                Color barColor = ColorFromHSV(hue, 0.85f, 0.85f);

                DrawRectangle(x, y, barWidth, height, barColor);
                
            }


            if (IsKeyPressed(KEY_N)) goToNextSong();
            if (IsKeyPressed(KEY_B)) goToPrevSong();


            if (IsKeyPressed(KEY_ESCAPE)) {
                StopSound(music);
                appState = STATE_MENU;
            }
            
            // for (size_t i = 0; i < FFT_SIZE / 2; i++)
            // {
            //     int magnitude = (int)(200.0f * log10f(magnitudes[i]));
            //     DrawLine(i, 600, i, 600 - magnitude, fg);
            // }
            
        }
        EndDrawing();
    }

    if (music.frameCount > 0) {
        DetachAudioStreamProcessor(music.stream, audioCallback);
        UnloadSound(music);
    }

    free(queue);
    

    CloseAudioDevice();
    CloseWindow();

}