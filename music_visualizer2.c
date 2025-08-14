#include <raylib.h>
#include "kissfft/kiss_fft.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <string.h>
#include "fft_queue.h"

#define FFT_SIZE 1024
#define MAX_SONGS 10

static char* str_dup(const char* s);
static char* str_dup(const char* s) {
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

kiss_fft_cpx in[FFT_SIZE];
kiss_fft_cpx out[FFT_SIZE];
fft_queue *queue;

float magnitudes[FFT_SIZE / 2];


const char *playlist[MAX_SONGS] = {
    "./1.mp3",
    "./2.mp3",
    "./3.mp3",
    "./5.mp3"
};

int initialPlaylistCount = 4;
int playlistCount = 4;
int selectedSong = 0;
int currentSong = -1;

Sound music = {0};

int right_panel_w = 250;
int top_pad = 20;
int left_pad = 20;
int row_h = 28; 

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

int main(int argc, char **argv) 
{

    queue = malloc(sizeof(fft_queue));
    queue->index = 0;

    InitWindow(800, 600, "music visualizer");
    InitAudioDevice();

    SetExitKey(KEY_NULL);

    music.frameCount = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose()) 
    {
        
        static double lastClickTime = 0.0;
        static int    lastClickIndex = -1;
        const double  DOUBLE_CLICK_TIME = 0.28; 

        Vector2 mp = GetMousePosition();
        Rectangle right_panel = (Rectangle){800 - right_panel_w, 0, right_panel_w, 600};

        if (CheckCollisionPointRec(mp, right_panel)) {
            int listTop = top_pad + 40;

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                int relY = (int)mp.y - listTop;
                if (relY >= 0) {
                    int idx = relY / row_h;
                    if (idx >= 0 && idx < playlistCount) {
                        double now = GetTime();

                        selectedSong = idx;

                        if (idx == lastClickIndex && (now - lastClickTime) < DOUBLE_CLICK_TIME) {
                            loadAndPlaySong(playlist[selectedSong]);
                            lastClickIndex = -1;
                            lastClickTime = 0.0;
                        } else {
                            lastClickIndex = idx;
                            lastClickTime = now;
                        }
                    }
                }
            }
        }

        if (IsFileDropped()) 
        {
            FilePathList dropped = LoadDroppedFiles();   
            for (int i = 0; i < dropped.count; i++) {
                const char *path = dropped.paths[i];

                if (!IsFileExtension(path, ".wav;.ogg;.mp3;.flac")) continue;

                bool exists = false;
                for (int j = 0; j < playlistCount; j++) {
                    if (playlist[j] && strcmp(playlist[j], path) == 0) { exists = true; break; }
                }
                if (exists) continue;

                if (playlistCount < MAX_SONGS) {
                    char *copy = str_dup(path);  
                    if (copy) {
                        playlist[playlistCount] = copy;
                        selectedSong = playlistCount;
                        playlistCount++;
                        loadAndPlaySong(playlist[selectedSong]); 
                    }
                } else {
                    TraceLog(LOG_WARNING, "Playlist dolu (MAX_SONGS=%d)", MAX_SONGS);
                }
            }
            UnloadDroppedFiles(dropped); 
        }

                
        BeginDrawing();
        {
            ClearBackground(BLACK);

            Rectangle left_panel = (Rectangle){0, 0, 800 - right_panel_w, 600};
            DrawRectangleRec(left_panel, (Color){0, 87, 48, 255});
            
            DrawText(TextFormat("Çalan: %s", (currentSong >= 0) ? playlist[currentSong] : "-"), 20, 20, 25, RAYWHITE);

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


            DrawText("Playlist", (int)right_panel.x + 14, top_pad, 22, RAYWHITE);

            int y = top_pad + 40;
            for (int i = 0; i < playlistCount; i++) 
            {
                Rectangle row = { right_panel.x + 10, (float)y, right_panel.width - 20, (float)row_h };
                bool sel = (i == selectedSong);
                if (sel) DrawRectangleRounded(row, 0.15f, 6, (Color){28,34,56,255});
                Color txt = sel ? (Color){200,215,255,255} : (Color){160,175,200,255};

                const char *name = playlist[i];
                int maxw = (int)row.width - 16;
                int tw = MeasureText(name, 18);
                if (tw <= maxw) 
                {
                    DrawText(name, (int)row.x + 8, (int)row.y + 4, 18, txt);
                } 
                else 
                {
                    static char buf[256];
                    strncpy(buf, name, sizeof(buf)-1);
                    buf[sizeof(buf)-1] = '\0';
                    while (MeasureText(buf, 18) > maxw - MeasureText("...", 18) && (int)strlen(buf) > 3)
                    {
                        buf[strlen(buf)-1] = '\0';

                    }
                    strcat(buf, "...");
                    DrawText(buf, (int)row.x + 8, (int)row.y + 4, 18, txt);
                }

                y += row_h;
            }


            if (IsKeyPressed(KEY_N)) 
                goToNextSong();
            if (IsKeyPressed(KEY_B)) 
                goToPrevSong();

            if (IsKeyPressed(KEY_DOWN)) 
                selectedSong = (selectedSong + 1) % playlistCount;
            if (IsKeyPressed(KEY_UP))
                selectedSong = (selectedSong - 1 + playlistCount) % playlistCount;

            if (IsKeyPressed(KEY_ENTER)) {
                if (selectedSong >= 0 && selectedSong < playlistCount) {
                    loadAndPlaySong(playlist[selectedSong]);
            }
            }
            if (IsKeyPressed(KEY_SPACE)) 
            {
                if(IsSoundPlaying(music)) 
                    StopSound(music);
                else if(currentSong >= 0) 
                    PlaySound(music);
            }
            
        }
        EndDrawing();
    }


    if (music.frameCount > 0)
    {
        DetachAudioStreamProcessor(music.stream, audioCallback);
        UnloadSound(music);
    }

    for (int i = initialPlaylistCount; i < playlistCount; i++) {
        free((void*)playlist[i]);
    }

    free(queue);
    

    CloseAudioDevice();
    CloseWindow();

    return 0;
    
}