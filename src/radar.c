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

#include "tokens.h"


// Radar Graphics
void DrawRadarOutline(void) {
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
void DrawSelectedPlaneInfo(void) {
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

void DrawRadarScaleIndicator(void) {
    float range_miles = GetRadarRangeMiles();
    float range_km = range_miles * 1.60934f;

    char scale_text[64];
    snprintf(scale_text, sizeof(scale_text), "View Window: ~%.1f mi (~%.1f km) wide", range_miles, range_km);

    int y_pos = WINDOW_HEIGHT - 85;
    DrawText(scale_text, 20, y_pos, 16, DARKGREEN);
}

void DrawErrorMsg(void) {
    const char* err_msg = "Could not receive data from OpenSky";
    int font_size = 38;
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.6f));
    const int text_width = MeasureText(err_msg, font_size);
    DrawText(err_msg, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - (font_size / 2), font_size, GREEN);
}

void DrawAirportMenu(void) {
    int total_airports = AIRPORT_DB_COUNT;

    int menu_width = 300;
    int menu_height = 420;
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

        char code[8];
        char name[64];

        if (i == 0) {
            strcpy(code, "HOME");
            strncpy(name, custom_name, sizeof(name));
        } else {
            strcpy(code, AIRPORT_DB[i - 1].code);
            strncpy(name, AIRPORT_DB[i - 1].name, sizeof(name));
        }

        if (is_hovered) {
            DrawRectangleRec(item_rect, (Color){ 30, 80, 30, 255 });
            if (mouse_clicked) {
                UpdateAirportBoundingBox(code);
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

        DrawText(TextFormat("[%s] %s", code, name), menu_x + 10, item_y + 2, 12, LIGHTGRAY);
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

// Setup Screen
char input_id[128] = {0};
char input_secret[128] = {0};
char input_lat[32] = "0.0000";
char input_lon[32] = "0.0000";
char input_name[64] = "My Location";

#define BORDER_WIDTH 2

static int active_field = 0;

void DrawSetupScreen(void) {
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.85));

    int box_w = 480, box_h = 460;
    int box_x = (WINDOW_WIDTH - box_w) / 2;
    int box_y = (WINDOW_HEIGHT - box_h) / 2;

    DrawRectangle(box_x, box_y, box_w, box_h, UI_BACKGROUND_COLOUR);
    DrawRectangleLines(box_x, box_y, box_w, box_h, GREEN);

    DrawText("FLIGHT RADAR SETUP", box_x + 20, box_y + 20, 18, YELLOW);
    DrawText("Press F1 to return", box_x + box_w - 140, box_y + 25, 10, LIGHTGRAY);

    DrawText("Visit 'https://opensky-network.org' to obtain an API ID & Secret", box_x + 20, box_y + 45, 10, LIGHTGRAY);

    // Input Fields
    Rectangle rect_id     = { (float)(box_x + 20), (float)(box_y + 85),  (float)(box_w - 40), 30 };
    Rectangle rect_secret = { (float)(box_x + 20), (float)(box_y + 150), (float)(box_w - 40), 30 };
    Rectangle rect_name   = { (float)(box_x + 20), (float)(box_y + 215), (float)(box_w - 40), 30 };
    Rectangle rect_lat    = { (float)(box_x + 20), (float)(box_y + 280), (float)(box_w / 2 - 30), 30 };
    Rectangle rect_lon    = { (float)(box_x + 250), (float)(box_y + 280), (float)(box_w / 2 - 30), 30 };
    Rectangle rect_save   = { (float)(box_x + 20), (float)(box_y + 350), (float)(box_w - 40), 40 };

    Vector2 mouse_pos = GetMousePosition();
    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (clicked) {
        if (CheckCollisionPointRec(mouse_pos, rect_id)) active_field = 1;
        else if (CheckCollisionPointRec(mouse_pos, rect_secret)) active_field = 2;
        else if (CheckCollisionPointRec(mouse_pos, rect_name)) active_field = 3;
        else if (CheckCollisionPointRec(mouse_pos, rect_lat)) active_field = 4;
        else if (CheckCollisionPointRec(mouse_pos, rect_lon)) active_field = 5;
        else active_field = 0;
    }

    if (IsKeyPressed(KEY_TAB)) {
        active_field = (active_field % 5) + 1;
    }

    char *target_buf = NULL;
    int max_len = 127;
    if (active_field == 1) { target_buf = input_id; max_len = sizeof(input_id) - 1; }
    else if (active_field == 2) { target_buf = input_secret; max_len = sizeof(input_secret) - 1; }
    else if (active_field == 3) { target_buf = input_name; max_len = sizeof(input_name) - 1; }
    else if (active_field == 4) { target_buf = input_lat; max_len = sizeof(input_lat) - 1; }
    else if (active_field == 5) { target_buf = input_lon; max_len = sizeof(input_lon) - 1; }

    // Copy Paste
    if (target_buf && ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                        IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) && IsKeyPressed(KEY_V))) {
        const char *clipboard = GetClipboardText();
        if (clipboard) {
            strncat(target_buf, clipboard, max_len - strlen(target_buf));
        }
                        }

    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 32) && (key <= 126) && target_buf) {
            int len = (int)strlen(target_buf);
            if (len < max_len) {
                target_buf[len] = (char)key;
                target_buf[len + 1] = '\0';
            }
        }
        key = GetCharPressed();
    }

    // Backspace
    if (target_buf) {
        static float backspace_timer = 0.0f;
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(target_buf);
            if (len > 0) target_buf[len - 1] = '\0';
            backspace_timer = 0.4f;
        } else if (IsKeyReleased(KEY_BACKSPACE)) {
            backspace_timer = 0.0f;
        } else if (IsKeyDown(KEY_BACKSPACE)) {
            backspace_timer -= GetFrameTime();
            if (backspace_timer <= 0.0f) {
                int len = strlen(target_buf);
                if (len > 0) target_buf[len - 1] = '\0';
                backspace_timer = 0.05f;
            }
        }
    }


    // Client ID Field
    DrawText("OpenSky Client ID:", box_x + 20, box_y + 65, 12, LIGHTGRAY);
    DrawRectangleRec(rect_id, (Color){ 20, 40, 20, 255 });
    DrawRectangleLinesEx(rect_id, BORDER_WIDTH, active_field == 1 ? YELLOW : DARKGREEN);
    DrawText(input_id, box_x + 28, box_y + 93, 14, GREEN);

    // Client Secret Field
    DrawText("OpenSky Client Secret:", box_x + 20, box_y + 130, 12, LIGHTGRAY);
    DrawRectangleRec(rect_secret, (Color){ 20, 40, 20, 255 });
    DrawRectangleLinesEx(rect_secret, BORDER_WIDTH, active_field == 2 ? YELLOW : DARKGREEN);
    DrawText(input_secret, box_x + 28, box_y + 158, 14, GREEN);

    // Custom Location Name Field
    DrawText("Location Name (e.g. Home):", box_x + 20, box_y + 195, 12, LIGHTGRAY);
    DrawRectangleRec(rect_name, (Color){ 20, 40, 20, 255 });
    DrawRectangleLinesEx(rect_name, BORDER_WIDTH, active_field == 3 ? YELLOW : DARKGREEN);
    DrawText(input_name, box_x + 28, box_y + 223, 14, GREEN);

    // Latitude Field
    DrawText("Latitude:", box_x + 20, box_y + 260, 12, LIGHTGRAY);
    DrawRectangleRec(rect_lat, (Color){ 20, 40, 20, 255 });
    DrawRectangleLinesEx(rect_lat, BORDER_WIDTH, active_field == 4 ? YELLOW : DARKGREEN);
    DrawText(input_lat, box_x + 28, box_y + 288, 14, GREEN);

    // Longitude Field
    DrawText("Longitude:", box_x + 250, box_y + 260, 12, LIGHTGRAY);
    DrawRectangleRec(rect_lon, (Color){ 20, 40, 20, 255 });
    DrawRectangleLinesEx(rect_lon, BORDER_WIDTH, active_field == 5 ? YELLOW : DARKGREEN);
    DrawText(input_lon, box_x + 258, box_y + 288, 14, GREEN);

    // Save & Connect Button
    bool save_hovered = CheckCollisionPointRec(mouse_pos, rect_save);
    DrawRectangleRec(rect_save, save_hovered ? (Color){ 40, 120, 40, 255 } : (Color){ 20, 80, 20, 255 });
    DrawRectangleLinesEx(rect_save, BORDER_WIDTH, GREEN);

    int text_w = MeasureText("SAVE & CONNECT", 16);
    DrawText("SAVE & CONNECT", box_x + (box_w - text_w) / 2, box_y + 362, 16, YELLOW);

    if (save_hovered && clicked) {
        if (strlen(input_id) > 0 && strlen(input_secret) > 0) {
            float lat = (float)atof(input_lat);
            float lon = (float)atof(input_lon);

            SaveConfig("config.txt", input_id, input_secret, lat, lon, input_name);

            UpdateAirportBoundingBox("HOME");
            SetWindowTitle(TextFormat("Flight Radar - %s", current_airport_name));

            RefreshToken();
            show_setup_screen = false;
        }
    }
}