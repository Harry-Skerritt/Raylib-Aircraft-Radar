#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "config.h"
#include "api.h"
#include "radar.h"
#include "airport.h"
#include "tokens.h"

// Todo: Add OAuth2 for more requests

// Global Vars
char current_airport[5] = "LHR";
char current_airport_name[64] = "London Heathrow";
float la_min, la_max, lo_min, lo_max;
SelectedPlane selected_plane = { .active = false };
char *cached_json = NULL;
float fetch_timer = 0.0f;
bool active = true;
bool show_airport_menu = false;


int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Flight Radar");
    if (!IsWindowReady()) return 1;
    SetTargetFPS(60);

    if (!LoadCredentials("/Volumes/Data/Code/Projects/C++/Radar/creds.txt")) {
        printf("Error: Could not load API creds. Exiting.\n");
        CloseWindow();
        return 1;
    }

    float angle = 0.0f;

    // Default
    UpdateAirportBoundingBox("LHR");
    SetWindowTitle(TextFormat("Flight Radar - %s", current_airport_name));

    // Auth Token
    // Todo: Show a page for user to set this up
    Token token = GetToken();


    while (!WindowShouldClose()) {
        // Update
        angle += GetFrameTime() * RADAR_SPEED;
        UpdatePlaneData();

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

            if (show_airport_menu && active) {
                DrawAirportMenu();
            }

            if (!active) {
                DrawErrorMsg();
            }

        EndDrawing();
    }

    if (cached_json) free(cached_json);
    CloseWindow();
    return 0;
}