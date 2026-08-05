#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "config.h"
#include "api.h"
#include "radar.h"
#include "airport.h"
#include "tokens.h"

// Global Vars
char current_airport[5] = "LHR";
char current_airport_name[64] = "London Heathrow";
float la_min, la_max, lo_min, lo_max;
SelectedPlane selected_plane = { .active = false };
char *cached_json = NULL;
float fetch_timer = 0.0f;
bool active = true;
bool show_airport_menu = false;
bool show_setup_screen = false;

float idle_timer = 0.0f;
const float IDLE_TIMEOUT = 300.0f; // 5 Mins
bool is_idle = false;

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flight Radar");
    if (!IsWindowReady()) return 1;
    SetTargetFPS(60);


    if (!LoadConfig()) {
        show_setup_screen = true;
    } else {
        strncpy(input_id, CLIENT_ID, sizeof(input_id));
        strncpy(input_secret, CLIENT_SECRET, sizeof(input_secret));
        snprintf(input_lat, sizeof(input_lat), "%.4f", custom_lat);
        snprintf(input_lon, sizeof(input_lon), "%.4f", custom_lon);
        strncpy(input_name, custom_name, sizeof(input_name));

        UpdateAirportBoundingBox("HOME");
    }

    float angle = 0.0f;
    SetWindowTitle(TextFormat("Flight Radar - %s", current_airport_name));

    if (!show_setup_screen) {
        GetToken();
    }


    while (!WindowShouldClose()) {
        // Update
        if (IsKeyPressed(KEY_F1)) {
            show_setup_screen = !show_setup_screen;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
        GetMouseDelta().x != 0.0f || GetMouseDelta().y != 0.0f) {
            idle_timer = 0.0f;
            if (is_idle) is_idle = false;
        } else {
            idle_timer += GetFrameTime();
        }

        bool window_active = IsWindowFocused();

        if (!show_setup_screen && idle_timer < IDLE_TIMEOUT) {
            is_idle = false;
            angle += GetFrameTime() * RADAR_SPEED;
            UpdatePlaneData();
        } else {
            is_idle = true;
        }

        // Draw
        BeginDrawing();
            ClearBackground(WINDOW_COLOUR);

            DrawRadarOutline();
            DrawRadarSpinner(angle);
            DrawPlanes(cached_json);
            DrawSelectedPlaneInfo();

            DrawAirport(current_airport);
            DrawAirportName();

            DrawRadarScaleIndicator();

            if (is_idle && !show_setup_screen) {
                DrawIdleMsg();
            }

            if (show_airport_menu && active && !show_setup_screen) {
                DrawAirportMenu();
            }

            if (show_setup_screen) {
                DrawSetupScreen();
            }

            if (!active && !show_setup_screen) {
                DrawErrorMsg();
            }

        EndDrawing();
    }

    if (cached_json) free(cached_json);
    CloseWindow();
    return 0;
}