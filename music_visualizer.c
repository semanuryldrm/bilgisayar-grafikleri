#include <raylib.h>
#include "kissfft/kiss_fft.h"
#include <stdio.h>
#include <stdlib.h>
#include "fft_queue.h"


#define FFT_SIZE 1024
kiss_fft_cpx in[FFT_SIZE];
kiss_fft_cpx out[FFT_SIZE];
fft_queue *queue;

float magnitudes[FFT_SIZE / 2];

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

int main(int argc, char **argv) {

    queue = malloc(sizeof(fft_queue));
    queue->index = 0;

    InitWindow(800, 600, "music visualizer");
    InitAudioDevice();

    if (!IsAudioDeviceReady())
    {
        perror("audio device is not ready!");
        exit(1);
    }

    Sound music = LoadSound("./5.mp3");
    if (!IsSoundValid(music))
    {
        perror("sound file is not valid!");
        exit(1);
    }

    PlaySound(music);

    AttachAudioStreamProcessor(music.stream, audioCallback);
    
    //Color bg = ColorFromHSV(110, 0.8, 0.7);
    //Color fg = ColorFromHSV(290, 0.8, 0.7);

    while (!WindowShouldClose())
    {
        ClearBackground(BLACK);
        BeginDrawing();
        {
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
            
            // for (size_t i = 0; i < FFT_SIZE / 2; i++)
            // {
            //     int magnitude = (int)(200.0f * log10f(magnitudes[i]));
            //     DrawLine(i, 600, i, 600 - magnitude, fg);
            // }
            
        }
        EndDrawing();
    }

    DetachAudioStreamProcessor(music.stream, audioCallback);

    CloseAudioDevice();

    free(queue);
}