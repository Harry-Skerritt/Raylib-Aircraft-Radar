//
// Created by Harry Skerritt on 05/08/2026.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cJSON.h>
#include "config.h"
#include "airport.h"
#include "radar.h"


// Radar Graphics
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
    for (int radius = 80; radius <= RADAR_CIRCLE_RAD; radius += 80) {
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

// Details
void DrawPlanes(const char *json_data) {
    if (!json_data) return;
    active = true;

    cJSON *root = cJSON_Parse(json_data);
    if (!root) return;

    int plane_count = 0;
    const cJSON *states = cJSON_GetObjectItemCaseSensitive(root, "states");
    if (cJSON_IsArray(states)) {
        const cJSON *plane = NULL;

        plane_count = cJSON_GetArraySize(states);

        Vector2 mouse_pos = GetMousePosition();
        bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        cJSON_ArrayForEach(plane, states) {
            cJSON *call_sign = cJSON_GetArrayItem(plane, 1);
            cJSON *longitude = cJSON_GetArrayItem(plane, 5);
            cJSON *latitude = cJSON_GetArrayItem(plane, 6);
            cJSON *baro_alt_m = cJSON_GetArrayItem(plane, 7);
            cJSON *from_apt = cJSON_GetArrayItem(plane, 2);

            if (cJSON_IsString(call_sign) && cJSON_IsNumber(longitude) && cJSON_IsNumber(latitude)) {
                float lon = (float)longitude->valuedouble;
                float lat = (float)latitude->valuedouble;

                float alt_ft = 0.0f;
                if (cJSON_IsNumber(baro_alt_m)) {
                    alt_ft = (float)baro_alt_m->valuedouble * MT_TO_FT;
                }

                Vector2 pos = MapGPSToRadar(lat, lon, la_min, la_max, lo_min, lo_max);
                bool is_hovered = CheckCollisionPointCircle(mouse_pos, pos, 8.0f);

                if (is_hovered && mouse_clicked) {
                    snprintf(selected_plane.callsign, sizeof(selected_plane.callsign), "%s", call_sign->valuestring);
                    selected_plane.lat = lat;
                    selected_plane.lon = lon;
                    selected_plane.alt_ft = alt_ft;
                    snprintf(selected_plane.from, sizeof(selected_plane.from), "%s", from_apt->valuestring);
                    selected_plane.active = true;
                }

                Color dot_color = (selected_plane.active && strcmp(selected_plane.callsign, call_sign->valuestring) == 0) ? YELLOW : GREEN;
                DrawCircleV(pos, is_hovered ? 6.0f : 4.0f, dot_color);
                DrawText(call_sign->valuestring, (int)pos.x + 6, (int)pos.y - 4, 10, LIGHTGRAY);
            }
        }
    }
    DrawText(TextFormat("Total Planes: %d", plane_count),10, 10, 20, GREEN);
    cJSON_Delete(root);
}

void DrawAirport(const char* airport_tag) {
    int text_size = MeasureText(airport_tag, 20);
    DrawText(airport_tag, WINDOW_WIDTH / 2 - text_size / 2, WINDOW_HEIGHT / 2 + 20, 20, GREEN);
}

// UI
void DrawSelectedPlaneInfo() {
    DrawRectangle(0, WINDOW_HEIGHT - 60, WINDOW_WIDTH, 60, UI_BACKGROUND_COLOUR);
    DrawRectangleLines(0, WINDOW_HEIGHT - 60, WINDOW_WIDTH, 60, RADAR_COLOUR);

    if (selected_plane.active) {
        DrawText(TextFormat("SELECTED FLIGHT: %s", selected_plane.callsign), 20, WINDOW_HEIGHT - 50, 16, YELLOW);
        DrawText(TextFormat("Latitude: %.4f", selected_plane.lat), 20, WINDOW_HEIGHT - 25, 14, LIGHTGRAY);
        DrawText(TextFormat("Longitude: %.4f", selected_plane.lon), 170, WINDOW_HEIGHT - 25, 14, LIGHTGRAY);

        if (selected_plane.alt_ft == 0.0f)
            DrawText(TextFormat("Altitude: - ft"), 320, WINDOW_HEIGHT - 25, 14, LIGHTGRAY);
        else
            DrawText(TextFormat("Altitude: %.0f ft", selected_plane.alt_ft), 320, WINDOW_HEIGHT - 25, 14, LIGHTGRAY);

        DrawText(TextFormat("From: %s", selected_plane.from), 470, WINDOW_HEIGHT - 25, 14, LIGHTGRAY);
    } else {
        DrawText("Click on a plane dot to view telemetry data...", 20, WINDOW_HEIGHT - 38, 14, DARKGREEN);
    }
}

void DrawRadarScaleIndicator() {
    float range_miles = GetRadarRangeMiles();
    float range_km = range_miles * 1.60934f;

    char scale_text[64];
    snprintf(scale_text, sizeof(scale_text), "View Window: ~%.1f mi (~%.1f km) wide", range_miles, range_km);

    int y_pos = WINDOW_HEIGHT - 85;
    DrawText(scale_text, 20, y_pos, 16, DARKGREEN);
}

void DrawErrorMsg() {
    const char* err_msg = "Could not receive data from OpenSky";
    int font_size = 38;
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.6f));
    const int text_width = MeasureText(err_msg, font_size);
    DrawText(err_msg, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - (font_size / 2), font_size, GREEN);
}

void DrawAirportMenu() {
    int total_airports = AIRPORT_DB_COUNT;

    int menu_width = 300;
    int menu_height = 400;
    int menu_x = WINDOW_WIDTH - menu_width - 10;
    int menu_y = 40;

    // Draw Background
    DrawRectangle(menu_x, menu_y, menu_width, menu_height, UI_BACKGROUND_COLOUR);
    DrawRectangleLines(menu_x, menu_y, menu_width, menu_height, GREEN);
    DrawText("SELECT AIRPORT:", menu_x + 10, menu_y + 10, 16, YELLOW);

    Vector2 mouse_pos = GetMousePosition();
    bool mouse_clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    int start_y = menu_y + 35;
    int item_height = 20;

    // Draw Menu + Handle Mouse
    for (int i = 0; i < total_airports; i++) {
        int item_y = start_y + (i * item_height);
        Rectangle item_rect = { (float)menu_x + 5, (float)item_y, (float)(menu_width - 10), (float)item_height };

        bool is_hovered = CheckCollisionPointRec(mouse_pos, item_rect);

        if (is_hovered) {
            DrawRectangleRec(item_rect, (Color){ 30, 80, 30, 255 });
            if (mouse_clicked) {
                UpdateAirportBoundingBox(AIRPORT_DB[i].code);
                SetWindowTitle(TextFormat("Flight Radar - %s", current_airport_name));
                show_airport_menu = false;

                fetch_timer = 10.0f;
                if (cached_json) {
                    free(cached_json);
                    cached_json = NULL;
                }
                break;
            }
        }

        DrawText(TextFormat("[%s] %s", AIRPORT_DB[i].code, AIRPORT_DB[i].name), menu_x + 10, item_y + 2, 12, LIGHTGRAY);
    }
}

void DrawAirportName(void) {
    char display_text[128];
    snprintf(display_text, sizeof(display_text), "%s", current_airport_name);

    int font_size = 20;
    int text_width = MeasureText(display_text, font_size);
    int x = WINDOW_WIDTH - text_width - 15;
    int y = 10;

    Rectangle btn_rect = { (float)(x - 5), (float)(y - 2), (float)(text_width + 10), (float)(font_size + 6) };
    bool is_hovered = CheckCollisionPointRec(GetMousePosition(), btn_rect);

    if (is_hovered) {
        DrawRectangleRec(btn_rect, (Color){ 20, 50, 20, 255 });
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            show_airport_menu = !show_airport_menu;
        }
    }

    DrawText(display_text, x, y, font_size, GREEN);
}