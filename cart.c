#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define FIFO_H_IMPLEMENTATION_
#include "fifo.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGTH 600




int main(int argc, char **argv)
{
    int r = 100;
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGTH, "cember");
    SetTargetFPS(60);
    Color bgd = ColorFromHSV(100, 0.8, 0.7);
    Color fgd = ColorFromHSV(280, 0.8, 0.7);
    
    float t = 0;
    float dt = 30;


    while(!WindowShouldClose())
    {
        ClearBackground(bgd);
        BeginDrawing();
        {


            float x1 = r * sinf(DEG2RAD * t / 10) + r / 5 * sinf(DEG2RAD * t / 5);
            float y1 = r * cosf(DEG2RAD * t / 10);

            float x2 = r * 2 * sinf(DEG2RAD * t / 10) + r / 50 * sinf(DEG2RAD * t);
            float y2 = r * 2 * cosf(DEG2RAD * t / 20) + r / 5 * cosf(DEG2RAD * t / 12);

            Raylib_Line rl = {
                .x1 = x1 + (SCREEN_WIDTH/2),
                .y1 = y1 + (SCREEN_HEIGTH/2),
                .x2 = x2 + (SCREEN_WIDTH/2),
                .y2 = y2 + (SCREEN_HEIGTH/2),
            };

            push(rline, rl);

            t += dt;

            DrawFPS(10, 10);
            for (size_t i = 0; i < FIFO_SIZE; i++)
            {
                DrawLine(rline[i].x1, rline[i].y1, rline[i].x2, rline[i].y2, fgd);
            }
            

            // for (int i = 0; i < 36000; i++)
            // {
            //     float x1 = r * sinf(DEG2RAD * i / 10) + r / 5 * sinf(DEG2RAD * i / 5);
            //     float y1 = r * cosf(DEG2RAD * i / 10);
            //     float x2 = r * 2 * sinf(DEG2RAD * i / 10) + r / 50 * sinf(DEG2RAD * i);
            //     float y2 = r * 2 * cosf(DEG2RAD * i / 20) + r / 5 * cosf(DEG2RAD * i / 12);

            //     DrawPixel(x1 + (SCREEN_WIDTH/2), y1 + (SCREEN_HEIGTH/2), fgd);
            //     DrawPixel(x2 + (SCREEN_WIDTH/2), y2 + (SCREEN_HEIGTH/2), fgd);

            //     //DrawLine(x1 + (SCREEN_WIDTH/2), y1 + (SCREEN_HEIGTH/2), x2 + (SCREEN_WIDTH/2), y2 + (SCREEN_HEIGTH/2), fgd);

            //     //DrawPixel(x1 + (SCREEN_WIDTH/2), y1 + (SCREEN_HEIGTH/2), fgd);
            // }
        }
        EndDrawing();

    }
    return 0;
}