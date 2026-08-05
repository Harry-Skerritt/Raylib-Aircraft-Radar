//
// Created by Harry Skerritt on 05/08/2026.
//

#ifndef AIRPORT_H
#define AIRPORT_H

#include <stdbool.h>
#include "raylib.h"

extern const Airport AIRPORT_DB[];
extern const int AIRPORT_DB_COUNT;

bool GetAirportCoords(const char* iata, float* out_lat, float* out_lon);
bool GetAirportName(const char* iata, char* out_name);
void UpdateAirportBoundingBox(const char* iata);

Vector2 MapGPSToRadar(float lat, float lon, float la_min, float la_max, float lo_min, float lo_max);
float GetRadarRangeMiles(void);

#endif //AIRPORT_H
