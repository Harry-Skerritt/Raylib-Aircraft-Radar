//
// Created by Harry Skerritt on 05/08/2026.
//

#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>
#include "raylib.h"

// Window Constants
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

// Colours
#define WINDOW_COLOUR (Color){ 16, 23, 16, 255 }
#define RADAR_COLOUR (Color){ 26, 105, 16, 255 }
#define UI_BACKGROUND_COLOUR (Color){ 10, 20, 10, 245 }

// Drawing Helpers
#define RADAR_PADDING 50
#define RADAR_SPEED 100
#define RADAR_TRAIL_COUNT 200
#define RADAR_CIRCLE_RAD 320

// Calcs
#define MT_TO_FT 3.281
#define AIRPORT_SPAN 0.18f


// Structs
typedef struct {
    char code[5]; // IATA
    char name[64]; // Full Name
    float lat;
    float lon;
} Airport;

typedef struct {
    char callsign[16];
    float lat;
    float lon;
    float alt_ft;
    char from[64];
    bool active;
} SelectedPlane;

// Shared Global Vars
extern char current_airport[5];
extern char current_airport_name[64];
extern float la_min, la_max, lo_min, lo_max;
extern SelectedPlane selected_plane;
extern char *cached_json;
extern float fetch_timer;
extern bool active;
extern bool show_airport_menu;
extern bool show_setup_screen;


#endif //CONFIG_H
