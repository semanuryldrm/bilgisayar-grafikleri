#include <stdio.h>
#include <raylib.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGTH 600

int main(int argc, char **argv)
{
    int r = 50;
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGTH, "cember");
    SetTargetFPS(60);
    Color bgd = ColorFromHSV(210, 0.8, 0.7);
    Color fgd = ColorFromHSV(30, 0.8, 0.7);
    
    while(!WindowShouldClose())
    {
        ClearBackground(bgd);
        BeginDrawing();
        {
            DrawFPS(10, 10);
            for (int i = 1; i < 360; i++)
            {
                float x1 = r * cosf(DEG2RAD * (i - 1));
                float y1 = r * sinf(DEG2RAD * (i - 1));
                float x2 = r * cosf(DEG2RAD * i);
                float y2 = r * sinf(DEG2RAD * i);

                DrawLine(x1 + (SCREEN_WIDTH/2), y1 + (SCREEN_HEIGTH/2), x2 + (SCREEN_WIDTH/2), y2 + (SCREEN_HEIGTH/2), fgd);

                //DrawPixel(x1 + (SCREEN_WIDTH/2), y1 + (SCREEN_HEIGTH/2), fgd);
            }
        }
        EndDrawing();

    }
    return 0;
}