//
// Created by Harry Skerritt on 05/08/2026.
//

#ifndef RADAR_H
#define RADAR_H

#include "raylib.h"

// Radar Graphic
void DrawRadarOutline(void);
void DrawRadarSpinner(const float current_angle);

// Details
void DrawPlanes(const char *json_data);
void DrawAirport(const char* airport_iata);

// UI
void DrawSelectedPlaneInfo(void);
void DrawRadarScaleIndicator(void);
void DrawErrorMsg(void);
void DrawAirportMenu(void);
void DrawAirportName(void);

// Setup Screen
extern char input_id[128];
extern char input_secret[128];
extern char input_lat[32];
extern char input_lon[32];
extern char input_name[64];
void DrawSetupScreen(void);

#endif //RADAR_H
