#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGTH 600

int main(int argc, char **argv)
{
    int r = 90;
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGTH, "lorenz");
    SetTargetFPS(60);
    Color bgd = ColorFromHSV(210, 0.8, 0.7);
    Color fgd = ColorFromHSV(30, 0.8, 0.7);

    Vector3 *points = malloc(100000 * sizeof(Vector3));

    Camera3D c = {
        .position = {
            .x = 0,
            .y = 0,
            .z = r
        },
        .fovy = 60,
        .projection = CAMERA_PERSPECTIVE,
        .target ={
            .x = 0,
            .y = 0,
            .z = 0
        },
        .up = {
            .x = 0,
            .y = 1,
            .z = 0
        }
    };

    float theta = 0, phi = 0;
    float d_theta = 0.001, d_phi = 0.0003;
    
    Vector3 start_pos = {.x = 0, .y = 0, .z = 0};

    points[0] = start_pos;
    int point_count = 1;

    float dt = 0.01;
    float a = 10.0, b = 8.0 / 3.0, p = 28.0;
    float x = 0.1, y = 0.0, z = 0.0;
    
    while(!WindowShouldClose())
    {
        //printf("cb\n");
        ClearBackground(bgd);

        theta += d_theta;
        phi += d_phi;

        c.position.x = r * sin(theta) * cos(phi);
        c.position.y = r * sin(theta) * sin(phi);
        c.position.z = r * cos(theta);

        float dx = (a * (y - x)) * dt;
        float dy = (x * (p - z) - y) * dt;
        float dz = ((x * y) - (b * z)) * dt;

        x += dx;
        y += dy;
        z += dz;

        Vector3 v = {.x = x, .y = y, .z = z};

        points[point_count++] = v;
        

        //printf("point_c: %d\n", point_count);

        BeginDrawing();
        {
            //DrawFPS(10, 10);
            BeginMode3D(c);

            for (size_t i = 0; i < point_count - 1; i++)
            {
                //printf("i1: %d\n", i);
                DrawLine3D(points[i], points[i + 1], fgd);
                //printf("point_c:%d i: %d, i + 1: %d\n", point_count, i, i+1);
            }

            EndMode3D();
            
        }
        EndDrawing();
    }
    return 0;
}