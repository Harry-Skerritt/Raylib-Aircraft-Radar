//
// Created by Harry Skerritt on 05/08/2026.
//

#include <strings.h>
#include "config.h"
#include "airport.h"

const Airport AIRPORT_DB[] = {
    { "LHR", "London Heathrow",          51.4700f,  -0.4543f },
    { "LGW", "London Gatwick",           51.1536f,  -0.1822f },
    { "STN", "London Stansted",          51.8850f,   0.2350f },
    { "LTN", "London Luton",             51.8747f,  -0.3683f },
    { "LCY", "London City",              51.5053f,   0.0553f },
    { "SEN", "Southend",                 51.5706f,   0.6936f },
    { "MAN", "Manchester",               53.3553f,  -2.2775f },
    { "BHX", "Birmingham",               52.4539f,  -1.7480f },
    { "BRS", "Bristol",                  51.3828f,  -2.7192f },
    { "NCL", "Newcastle",                55.0380f,  -1.6896f },
    { "LPL", "Liverpool",                53.3336f,  -2.8497f },
    { "LBA", "Leeds Bradford",           53.8658f,  -1.6606f },
    { "EMA", "East Midlands",            52.8311f,  -1.3281f },
    { "BOH", "Bournemouth",              50.7805f,  -1.8396f },
    { "EXT", "Exeter",                   50.7344f,  -3.4139f },
    { "MME", "Teesside International",   54.5092f,  -1.4294f },

    { "ORM", "Sywell Aerodrome",         52.3053f,  -0.7922f },
    { "EGTH", "Old Warden Aerodrome",    52.0867f,  -0.3186f }
};

const int AIRPORT_DB_COUNT = sizeof(AIRPORT_DB) / sizeof(AIRPORT_DB[0]);

bool GetAirportCoords(const char* iata, float *out_lat, float *out_lon) {
    int count = sizeof(AIRPORT_DB) / sizeof(AIRPORT_DB[0]);
    for (int i = 0; i < count; i++) {
        if (strcasecmp(AIRPORT_DB[i].code, iata) == 0) {
            *out_lat = AIRPORT_DB[i].lat;
            *out_lon = AIRPORT_DB[i].lon;
            return true;
        }
    }
    return false; // Not Found
}

bool GetAirportName(const char* iata, char* out_name) {
    int count = sizeof(AIRPORT_DB) / sizeof(AIRPORT_DB[0]);
    for (int i = 0; i < count; i++) {
        if (strcasecmp(AIRPORT_DB[i].code, iata) == 0) {
            strcpy(out_name, AIRPORT_DB[i].name);
            return true;
        }
    }
    return false; // Not Found
}

void UpdateAirportBoundingBox(const char *icao) {
    float center_lat, center_lon;

    if (GetAirportCoords(icao, &center_lat, &center_lon)) {
        strncpy(current_airport, icao, 4);
        if (!GetAirportName(icao, current_airport_name)) {
            strncpy(current_airport_name, "Unknown Name", 64);
        }
    } else {
        TraceLog(LOG_WARNING, "Airport %s not found in DB, defaulting to LHR", icao);
        center_lat = 51.4700;
        center_lon = -0.4543f;
    }

    la_min = center_lat - AIRPORT_SPAN;
    la_max = center_lat + AIRPORT_SPAN;
    lo_min = center_lon - AIRPORT_SPAN;
    lo_max = center_lon + AIRPORT_SPAN;
}

// General
Vector2 MapGPSToRadar(float lat, float lon, float lamin, float lamax, float lomin, float lomax) {
    float cx = WINDOW_WIDTH / 2.0f;
    float cy = WINDOW_HEIGHT / 2.0f;

    float norm_x = (lon - lomin) / (lomax - lomin);
    float norm_y = (lat - lamin) / (lamax - lamin);

    float screen_x = cx + (norm_x - 0.5f) * (RADAR_CIRCLE_RAD * 2.0f);
    float screen_y = cy - (norm_y - 0.5f) * (RADAR_CIRCLE_RAD * 2.0f);

    return (Vector2){ screen_x, screen_y };
}

float GetRadarRangeMiles() {
    float total_span_degrees = AIRPORT_SPAN * 2.0f;
    float miles_per_degree_lat = 69.0f;
    return total_span_degrees * miles_per_degree_lat;
}
