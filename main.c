#include <stdio.h>
#include <math.h>
#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define WINDOW_COLOUR (Color){ 16, 23, 16, 255 }
#define RADAR_COLOUR (Color){ 26, 105, 16, 255 }

#define RADAR_PADDING 50
#define RADAR_SPEED 100
#define RADAR_TRAIL_COUNT 200

void DrawRadarOutline() {
    const float cx = WINDOW_WIDTH / 2.0f;
    const float cy = WINDOW_HEIGHT / 2.0f;
    // Draw Lines (X/Y)
    DrawLine(RADAR_PADDING, (int)cy, WINDOW_WIDTH - RADAR_PADDING, (int)cy, RADAR_COLOUR);
    DrawLine((int)cx, RADAR_PADDING, (int)cx, WINDOW_HEIGHT - RADAR_PADDING, RADAR_COLOUR);

    // Draw Lines 45deg
    const float length = 450.0f;
    const float cos_val = cosf(45.0f * DEG2RAD) * length;
    const float sin_val = sinf(45.0f * DEG2RAD) * length;

    DrawLineV((Vector2){ cx - cos_val, cy - sin_val }, (Vector2){ cx + cos_val, cy + sin_val }, RADAR_COLOUR);
    DrawLineV((Vector2){ cx - cos_val, cy + sin_val }, (Vector2){ cx + cos_val, cy - sin_val }, RADAR_COLOUR);

    // Draw Circles
    for (int radius = 80; radius <= 320; radius += 80) {
        DrawCircleLines((int)cx, (int)cy, (float)radius, RADAR_COLOUR);
    }
}

void DrawRadarSpinner(const float current_angle) {
    const float length = 450.0f;
    const Vector2 centre = (Vector2){ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f };

    float angle_step = .2f;

    // Trail
    for (int i = 0; i < RADAR_TRAIL_COUNT; i++) {
        const float trail_angle = current_angle - ((float)i * angle_step);
        const float alpha_factor = 1.0f - ((float)i / (float)RADAR_TRAIL_COUNT);
        const unsigned char alpha = (unsigned char)(255.0f * alpha_factor * 0.4f);

        const Color fade_color = (Color){ 0, 255, 0, alpha };

        const float cos_val = cosf(trail_angle * DEG2RAD) * length;
        const float sin_val = sinf(trail_angle * DEG2RAD) * length;

        DrawLineV(centre, (Vector2){ centre.x + cos_val, centre.y + sin_val }, fade_color);
    }

    // Main Line
    const float main_cos = cosf(current_angle * DEG2RAD) * length;
    const float main_sin = sinf(current_angle * DEG2RAD) * length;
    DrawLineV(centre, (Vector2){ centre.x + main_cos, centre.y + main_sin }, GREEN);
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Plane Radar");
    if (!IsWindowReady()) return 1;
    SetTargetFPS(0);

    float angle = 0.0f;

    while (!WindowShouldClose()) {
        // Update
        angle += GetFrameTime() * RADAR_SPEED;

        // Draw
        BeginDrawing();
        DrawFPS(10, 10);
        ClearBackground(WINDOW_COLOUR);

        DrawRadarOutline();
        DrawRadarSpinner(angle);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}