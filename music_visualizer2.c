// music_visualizer2.c — PREV & NEXT: aynı RT'den, aynı ölçekte (ICON_SCALE=1.00)
#include <math.h>
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fft_queue.h"
#include "kissfft/kiss_fft.h"

#define FFT_SIZE 1024
#define MAX_SONGS 10
#define FULL_WIDTH_BAR 0  // 1 yaparsan alt bar tüm genişlikte olur

static int WINDOW_W = 800, WINDOW_H = 600;
int right_panel_w = 250, top_pad = 20, row_h = 28;

const int bottom_bar_h = 72, controls_y_pad = 20, progress_h = 6;
int btn_r = 20;

// İkon geometri ve ölçek
int ICON_H = 28, ICON_W = 22;
const float ICON_SCALE = 1.00f;  // <-- PREV & NEXT aynı

// --- RENDER TEXTURE (ikon için)
RenderTexture2D gIconRT;
int ICON_RT_W = 64, ICON_RT_H = 64;

static char *str_dup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}
static void fmt_time(float sec, char *buf, int n) {
    if (sec < 0) sec = 0;
    int s = (int)roundf(sec);
    snprintf(buf, n, "%d:%02d", s / 60, s % 60);
}
static bool pointInCircle(Vector2 p, Vector2 c, float r) {
    float dx = p.x - c.x, dy = p.y - c.y;
    return dx * dx + dy * dy <= r * r;
}

// ------- FFT -------
kiss_fft_cpx in[FFT_SIZE], out[FFT_SIZE];
float magnitudes[FFT_SIZE / 2];
static kiss_fft_cfg g_fft_cfg = NULL;
static float g_hann[FFT_SIZE];
static void init_hann(void) {
    for (int i = 0; i < FFT_SIZE; i++)
        g_hann[i] = 0.5f - 0.5f * cosf(2.0f * PI * i / (FFT_SIZE - 1));
}

fft_queue *queue;

const char *playlist[MAX_SONGS] = {"./1.mp3", "./2.mp3", "./3.mp3", "./5.mp3"};
int initialPlaylistCount = 4, playlistCount = 4, selectedSong = 0, currentSong = -1;

Music music = {0};

static void run_fft(fft_queue *q) {
    for (int i = 0; i < FFT_SIZE; i++) {
        in[i].r = q->samples[i] * g_hann[i];
        in[i].i = 0.0f;
    }
    kiss_fft(g_fft_cfg, in, out);
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        float re = out[i].r, im = out[i].i;
        float mag = sqrtf(re * re + im * im) / (FFT_SIZE * 0.5f);
        float db = 20.0f * log10f(mag + 1e-8f);
        float v = (db + 80.0f) / 80.0f;
        if (v < 0) v = 0;
        if (v > 1) v = 1;
        magnitudes[i] = 0.85f * magnitudes[i] + 0.15f * v;
    }
}

static void audioCallback(void *buffer, unsigned int frames) {
    float *s = (float *)buffer;
    for (unsigned int i = 0; i < frames; i++) {
        float mono = (s[2 * i] + s[2 * i + 1]) * 0.5f;
        push_fft_queue(queue, mono);
        if (queue->index >= FFT_SIZE) {
            run_fft(queue);
            clear_fft_queue(queue);
        }
    }
}

static void loadAndPlaySong(const char *fn) {
    if (music.ctxData != NULL) {
        DetachAudioStreamProcessor(music.stream, audioCallback);
        StopMusicStream(music);
        UnloadMusicStream(music);
        music.ctxData = NULL;
    }
    music = LoadMusicStream(fn);
    if (music.ctxData == NULL) {
        TraceLog(LOG_ERROR, "Failed to load: %s", fn);
        return;
    }
    AttachAudioStreamProcessor(music.stream, audioCallback);
    PlayMusicStream(music);
    currentSong = selectedSong;
    clear_fft_queue(queue);
}
static void goToNextSong(void) {
    selectedSong = (currentSong + 1) % playlistCount;
    loadAndPlaySong(playlist[selectedSong]);
}
static void goToPrevSong(void) {
    selectedSong = (currentSong - 1 + playlistCount) % playlistCount;
    loadAndPlaySong(playlist[selectedSong]);
}

// ---- üçgen yardımcıları ----
static void DrawTriFilledOutlined(Vector2 A, Vector2 B, Vector2 C, Color fill, Color outline) {
    DrawTriangle(A, B, C, fill);
    DrawTriangleLines(A, B, C, outline);
}
static void DrawTriangleRight_Centered(Vector2 c, float w, float h, Color fill, Color outline) {
    Vector2 A = {c.x - w * 0.5f, c.y - h * 0.5f};
    Vector2 B = {c.x - w * 0.5f, c.y + h * 0.5f};
    Vector2 C = {c.x + w * 0.5f, c.y};
    DrawTriFilledOutlined(A, B, C, fill, outline);
}

// --- NEXT ikonunu render texture’a çiz (her kare) ---
static void DrawNextIconToRT(void) {
    BeginTextureMode(gIconRT);
    ClearBackground(BLANK);
    Color F = WHITE, O = (Color){0, 0, 0, 180};
    Vector2 center = {(float)(ICON_RT_W / 2), (float)(ICON_RT_H / 2)};
    // NEXT ofsetleri: -8 ve +6
    DrawTriangleRight_Centered((Vector2){center.x - 8, center.y}, ICON_W, ICON_H, F, O);
    DrawTriangleRight_Centered((Vector2){center.x + 6, center.y}, ICON_W, ICON_H, F, O);
    EndTextureMode();
}

// ---- ALT BAR ----
static void DrawMediaBar(Music m, int windowW, int windowH, int rightPanelW) {
    int barW = FULL_WIDTH_BAR ? windowW : (windowW - rightPanelW);
    Rectangle bottom_bar = {0, (float)(windowH - bottom_bar_h), (float)barW, (float)bottom_bar_h};
    DrawRectangleRec(bottom_bar, (Color){12, 12, 16, 255});

    // süreler
    float total = 0, cur = 0;
    if (m.ctxData != NULL) {
        total = GetMusicTimeLength(m);
        cur = GetMusicTimePlayed(m);
        if (cur < 0) cur = 0;
        if (total > 0 && cur > total) cur = total;
    }
    char L[16], R[16];
    fmt_time(cur, L, sizeof(L));
    fmt_time(total, R, sizeof(R));
    int ty = (int)(bottom_bar.y + bottom_bar_h - 40);
    DrawText(L, 20, ty, 18, (Color){180, 200, 220, 255});
    int rw = MeasureText(R, 18);
    DrawText(R, barW - 20 - rw, ty, 18, (Color){180, 200, 220, 255});

    // progress
    int pl = 20, pr = barW - 20, py = (int)(bottom_bar.y + controls_y_pad + 28);
    Rectangle pbg = {(float)pl, (float)py, (float)(pr - pl), (float)progress_h};
    DrawRectangleRounded(pbg, 0.5f, 4, (Color){60, 66, 78, 255});
    float ratio = (total > 0) ? (cur / total) : 0;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;
    Rectangle pfg = pbg;
    pfg.width = pbg.width * ratio;
    DrawRectangleRounded(pfg, 0.5f, 4, (Color){140, 190, 255, 255});
    int hx = (int)(pbg.x + pfg.width), hy = (int)(pbg.y + progress_h / 2);
    DrawCircle(hx, hy, 6, (Color){230, 240, 255, 255});

    // seek
    Vector2 mp = GetMousePosition();
    bool over = mp.x >= pbg.x && mp.x <= pbg.x + pbg.width && mp.y >= pbg.y - 8 && mp.y <= pbg.y + pbg.height + 8;
    static bool seeking = false;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && over) seeking = true;
    if (seeking) {
        float cx = mp.x;
        if (cx < pbg.x) cx = pbg.x;
        if (cx > pbg.x + pbg.width) cx = pbg.x + pbg.width;
        float r = (cx - pbg.x) / pbg.width;
        Rectangle tmp = pbg;
        tmp.width = (cx - pbg.x);
        DrawRectangleRounded(tmp, 0.5f, 4, (Color){180, 220, 255, 255});
        DrawCircle((int)cx, hy, 7, WHITE);
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && m.ctxData != NULL && total > 0) {
            SeekMusicStream(m, r * total);
            seeking = false;
        }
    }
    if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) seeking = false;

    // buton merkezleri
    int cx = barW / 2, cy = (int)(bottom_bar.y + controls_y_pad);
    Vector2 prevC = {(float)(cx - 150), (float)cy};
    Vector2 playC = {(float)(cx), (float)cy};
    Vector2 nextC = {(float)(cx + 150), (float)cy};

    // hover
    bool hovPrev = pointInCircle(mp, prevC, btn_r);
    bool hovPlay = pointInCircle(mp, playC, btn_r + 6);
    bool hovNext = pointInCircle(mp, nextC, btn_r);

    // arkaplanlar
    Color btnBg = (Color){24, 28, 38, 255};
    Color btnBg2 = (Color){34, 40, 56, 255};
    Color btnHover = (Color){38, 46, 62, 255};
    DrawCircleV(prevC, btn_r, hovPrev ? btnHover : btnBg);
    DrawCircleV(playC, btn_r + 6, hovPlay ? btnHover : btnBg2);
    DrawCircleV(nextC, btn_r, hovNext ? btnHover : btnBg);

    // renkler
    Color F = WHITE, O = (Color){0, 0, 0, 180};

    // --- PLAY/PAUSE ---
    if (m.ctxData != NULL && IsMusicStreamPlaying(m)) {
        DrawRectangle((int)(playC.x - 10), (int)(playC.y - 15), 8, 30, O);
        DrawRectangle((int)(playC.x + 2), (int)(playC.y - 15), 8, 30, O);
        DrawRectangle((int)(playC.x - 10), (int)(playC.y - 14), 8, 28, F);
        DrawRectangle((int)(playC.x + 2), (int)(playC.y - 14), 8, 28, F);
    } else {
        DrawTriangleRight_Centered((Vector2){playC.x + 2, playC.y}, ICON_W + 6, ICON_H + 6, F, O);
    }

    // --- Render texture'ı güncelle (NEXT'i içine çiz) ---
    DrawNextIconToRT();

    // --- NEXT (sağ): RT'den (aynı ölçekte) ---
    {
        Rectangle src = {0, 0, (float)gIconRT.texture.width, (float)gIconRT.texture.height};
        Rectangle dst = {
            nextC.x - (ICON_RT_W * ICON_SCALE) / 2.0f,
            nextC.y - (ICON_RT_H * ICON_SCALE) / 2.0f,
            ICON_RT_W * ICON_SCALE,
            ICON_RT_H * ICON_SCALE};
        DrawTexturePro(gIconRT.texture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    }

    // --- PREV (sol): RT'yi yatay ters çiz (aynı ölçekte) ---
    {
        Rectangle src = {0, 0, -(float)gIconRT.texture.width, (float)gIconRT.texture.height};  // X'te eksi -> mirror
        Rectangle dst = {
            prevC.x - (ICON_RT_W * ICON_SCALE) / 2.0f,
            prevC.y - (ICON_RT_H * ICON_SCALE) / 2.0f,
            ICON_RT_W * ICON_SCALE,
            ICON_RT_H * ICON_SCALE};
        DrawTexturePro(gIconRT.texture, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    }

    // tıklamalar
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (hovPlay) {
            if (m.ctxData != NULL) {
                if (IsMusicStreamPlaying(m))
                    PauseMusicStream(m);
                else if (currentSong >= 0)
                    ResumeMusicStream(m);
            }
        } else if (hovPrev) {
            goToPrevSong();
        } else if (hovNext) {
            goToNextSong();
        }
    }
}

int main(void) {
    queue = (fft_queue *)malloc(sizeof(fft_queue));
    memset(queue, 0, sizeof(fft_queue));
    InitWindow(WINDOW_W, WINDOW_H, "music visualizer");
    InitAudioDevice();
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    // ikon RT
    gIconRT = LoadRenderTexture(ICON_RT_W, ICON_RT_H);

    g_fft_cfg = kiss_fft_alloc(FFT_SIZE, 0, NULL, NULL);
    init_hann();
    for (int i = 0; i < FFT_SIZE / 2; i++) magnitudes[i] = 0.0f;
    music.ctxData = NULL;

    while (!WindowShouldClose()) {
        WINDOW_W = GetScreenWidth();
        WINDOW_H = GetScreenHeight();

        if (music.ctxData != NULL) {
            UpdateMusicStream(music);
            float t = GetMusicTimePlayed(music), T = GetMusicTimeLength(music);
            if (T > 0 && (T - t) < 0.01f) goToNextSong();
        }

        // sağ liste (seç/çift tık)
        static double lastClickTime = 0.0;
        static int lastClickIndex = -1;
        const double DC = 0.28;
        Vector2 mp = GetMousePosition();
        Rectangle right_panel = {(float)(WINDOW_W - right_panel_w), 0, (float)right_panel_w, (float)WINDOW_H};
        if (CheckCollisionPointRec(mp, right_panel) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int relY = (int)mp.y - (top_pad + 40);
            if (relY >= 0) {
                int idx = relY / row_h;
                if (idx >= 0 && idx < playlistCount) {
                    double now = GetTime();
                    selectedSong = idx;
                    if (idx == lastClickIndex && (now - lastClickTime) < DC) {
                        loadAndPlaySong(playlist[selectedSong]);
                        lastClickIndex = -1;
                        lastClickTime = 0;
                    } else {
                        lastClickIndex = idx;
                        lastClickTime = now;
                    }
                }
            }
        }

        // sürükle-bırak
        if (IsFileDropped()) {
            FilePathList dropped = LoadDroppedFiles();
            for (int i = 0; i < dropped.count; i++) {
                const char *path = dropped.paths[i];
                if (!IsFileExtension(path, ".wav;.ogg;.mp3;.flac")) continue;
                bool exists = false;
                for (int j = 0; j < playlistCount; j++) {
                    if (playlist[j] && strcmp(playlist[j], path) == 0) {
                        exists = true;
                        break;
                    }
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

        // klavye
        if (IsKeyPressed(KEY_N)) goToNextSong();
        if (IsKeyPressed(KEY_B)) goToPrevSong();
        if (IsKeyPressed(KEY_DOWN)) selectedSong = (selectedSong + 1) % playlistCount;
        if (IsKeyPressed(KEY_UP)) selectedSong = (selectedSong - 1 + playlistCount) % playlistCount;
        if (IsKeyPressed(KEY_ENTER)) {
            if (selectedSong >= 0 && selectedSong < playlistCount)
                loadAndPlaySong(playlist[selectedSong]);
        }
        if (IsKeyPressed(KEY_SPACE)) {
            if (music.ctxData != NULL) {
                if (IsMusicStreamPlaying(music))
                    PauseMusicStream(music);
                else if (currentSong >= 0)
                    ResumeMusicStream(music);
            }
        }

        // çizim
        BeginDrawing();
        {
            ClearBackground(BLACK);

            int vizW = WINDOW_W - right_panel_w;
            DrawRectangleRec((Rectangle){0, 0, (float)vizW, (float)WINDOW_H}, (Color){0, 87, 48, 255});
            DrawText(TextFormat("Çalan: %s", (currentSong >= 0) ? playlist[currentSong] : "-"), 20, 20, 25, RAYWHITE);

            int usableH = WINDOW_H - bottom_bar_h - 40, x0 = 20, yBase = 40 + usableH;
            int barWidth = 3, spacing = 1;
            int totalBars = (vizW - 2 * x0) / (barWidth + spacing);
            if (totalBars > (FFT_SIZE / 2)) totalBars = FFT_SIZE / 2;
            for (int i = 0; i < totalBars; i++) {
                int idx = i * ((FFT_SIZE / 2) / totalBars);
                float v = magnitudes[idx];
                int h = (int)(v * (float)usableH);
                if (h < 0) h = 0;
                int x = x0 + i * (barWidth + spacing), y = yBase - h;
                Color c = ColorFromHSV(360.0f * i / totalBars, 0.85f, 0.85f);
                DrawRectangle(x, y, barWidth, h, c);
            }

            DrawText("Playlist", (int)(WINDOW_W - right_panel_w) + 14, top_pad, 22, RAYWHITE);
            int y = top_pad + 40;
            for (int i = 0; i < playlistCount; i++) {
                Rectangle row = {(float)(WINDOW_W - right_panel_w + 10), (float)y, (float)right_panel_w - 20, (float)row_h};
                bool sel = (i == selectedSong);
                if (sel) DrawRectangleRounded(row, 0.15f, 6, (Color){28, 34, 56, 255});
                Color txt = sel ? (Color){200, 215, 255, 255} : (Color){160, 175, 200, 255};
                const char *name = playlist[i];
                int maxw = (int)row.width - 16, tw = MeasureText(name, 18);
                if (tw <= maxw) {
                    DrawText(name, (int)row.x + 8, (int)row.y + 4, 18, txt);
                } else {
                    static char buf[256];
                    strncpy(buf, name, sizeof(buf) - 1);
                    buf[sizeof(buf) - 1] = '\0';
                    while (MeasureText(buf, 18) > maxw - MeasureText("...", 18) && (int)strlen(buf) > 3)
                        buf[strlen(buf) - 1] = '\0';
                    strcat(buf, "...");
                    DrawText(buf, (int)row.x + 8, (int)row.y + 4, 18, txt);
                }
                y += row_h;
            }

            DrawMediaBar(music, WINDOW_W, WINDOW_H, right_panel_w);
        }
        EndDrawing();
    }

    if (music.ctxData != NULL) {
        DetachAudioStreamProcessor(music.stream, audioCallback);
        StopMusicStream(music);
        UnloadMusicStream(music);
    }
    for (int i = initialPlaylistCount; i < playlistCount; i++) free((void *)playlist[i]);
    if (g_fft_cfg) free(g_fft_cfg);
    free(queue);

    // RT temizliği
    UnloadRenderTexture(gIconRT);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
