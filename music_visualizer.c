#include <raylib.h>
#include <stdio.h>
#include "kissfft/kiss_fft.h"

#define FFT_SIZE 1024

static float input[FFT_SIZE];
static float magnitudes[FFT_SIZE/2];
static kiss_fft_cfg kissConfig;
static kiss_fft_cpx fft_in[FFT_SIZE];
static kiss_fft_cpx fft_out[FFT_SIZE];

static float exponent = 1.0f;                 // Audio exponentiation value

void ProcessAudio(void *buffer, unsigned int frames)
{
    float *samples = (float *)buffer;   // Samples internally stored as <float>s

    if (frames < FFT_SIZE) return;

    for (int i = 0; i < FFT_SIZE; i++)
    {
        float left = samples[i * 2];
        input[i] = left;

        fft_in[i].r = input[i];
        fft_in[i].i = 0.0f;
    }

    kiss_fft(kissConfig, fft_in, fft_out);

    for (int i = 0; i < FFT_SIZE / 2; i++)
    {
        float re = fft_out[i].r;
        float im = fft_out[i].i;
        magnitudes[i] = sqrtf(re * re + im * im);
    }
    
    
        
    

    for (unsigned int frame = 0; frame < frames; frame++)
    {
        //printf("frame: %d\n", frame);

        float *left = &samples[frame * 2 + 0], *right = &samples[frame * 2 + 1];

        *left = powf(fabsf(*left), exponent) * ( (*left < 0.0f)? -1.0f : 1.0f );
        *right = powf(fabsf(*right), exponent) * ( (*right < 0.0f)? -1.0f : 1.0f );

    }
}


int main(int argc, char **argv) {


    InitWindow(800, 600, "audio_visualizer");
    InitAudioDevice();

    AttachAudioMixedProcessor(ProcessAudio);

    Wave w = LoadWave("/home/sny/Downloads/memphis.wav");

    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    
    SetTargetFPS(60);

    kissConfig = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);

    // for (int i = 0; i < FFT_SIZE / 2; i++) {
    //     magnitudes[i] = 100.0f; // test verisi
    // }


    PlaySound(s);

    Color bg = ColorFromHSV(110, 0.8, 0.7);
    ClearBackground(bg);

    while (!WindowShouldClose())
    {
       
        BeginDrawing();
        ClearBackground(BLACK);
        {
            int barCount = 64;
            float minFreq = 20.0f;
            float maxFreq = 20000.0f;
            float sampleRate = 44100.0f;

            float logMin = log10f(minFreq);
            float logMax = log10f(maxFreq);

            for (int i = 0; i < barCount; i++)
            {
                float t = (float)i / (float)(barCount - 1);
                float freq = powf(10.0f, logMin + t * (logMax - logMin));
                int bin = (int)(freq * FFT_SIZE / sampleRate);
                if (bin >= FFT_SIZE / 2) bin = FFT_SIZE / 2 - 1;

                float mag = magnitudes[bin] * 0.05f;
                if (mag > 300) mag = 300;

                float barWidth = 800.0f / barCount;
                float barX = i * barWidth;

                Color c = ColorFromHSV(t * 300, 1.0f, 0.8f);
                DrawRectangle(barX, 600 - mag, barWidth - 1, mag, c);
            }
            DrawText("Press ESC to exit", 10, 10, 20, RAYWHITE);

        }
        EndDrawing();
    }
    
    UnloadSound(s);
    free(kissConfig);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}