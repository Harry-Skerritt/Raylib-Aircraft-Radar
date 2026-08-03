#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cJSON.h>
#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define WINDOW_COLOUR (Color){ 16, 23, 16, 255 }
#define RADAR_COLOUR (Color){ 26, 105, 16, 255 }

#define RADAR_PADDING 50
#define RADAR_SPEED 100
#define RADAR_TRAIL_COUNT 200
#define RADAR_CIRCLE_RAD 320

#define MT_TO_FT 3.281

#define LA_MIN 51.0698
#define LA_MAX 51.8823
#define LO_MIN -1.2271
#define LO_MAX 0.2479

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

// CURL
struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(const void *contents, const size_t size, const size_t nmemb, void *user) {
    size_t real_size = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)user;

    char *ptr = realloc(mem->memory, mem->size + real_size + 1);
    if (!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, real_size);
    mem->size += real_size;
    mem->memory[mem->size] = 0;

    return real_size;
}

char* GetEndpoint(const char *url) {
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();

    if (!curl) {
        fprintf(stderr, "Curl Init Failed");
        free(chunk.memory);
        chunk.memory = NULL;
        curl_global_cleanup();
        return chunk.memory;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "C-Radar-App/1.0");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    const CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        chunk.memory = NULL;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return chunk.memory;
}

// JSON Parsing
static float fetch_timer = 0.0f;
static char *cached_json = NULL;

void UpdatePlaneData() {
    fetch_timer += GetFrameTime();

    if (fetch_timer < 10.0f && cached_json != NULL) {
        return;
    }
    fetch_timer = 0.0f;


    char url[512];
    snprintf(url, sizeof(url),
             "https://opensky-network.org/api/states/all?lamin=%.4f&lamax=%.4f&lomin=%.4f&lomax=%.4f",
             LA_MIN, LA_MAX, LO_MIN, LO_MAX);

    printf("URL: %s\n", url);

    printf("Fetching aircraft data!\n");
    char* new_json = GetEndpoint(url);

    if (!new_json) {
        printf("Failed to fetch aircraft data.\n");
        return;
    }

    if (cached_json) {
        free(cached_json);
    }
    cached_json = new_json;

    cJSON *root = cJSON_Parse(cached_json);
    if (!root) {
        printf("Failed to parse JSON.\n");
        return;
    }

    cJSON_Delete(root);
}

// Raylib
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

void DrawPlanes(const char *json_data) {
    if (!json_data) return;

    cJSON *root = cJSON_Parse(json_data);
    if (!root) return;

    cJSON *states = cJSON_GetObjectItemCaseSensitive(root, "states");
    if (cJSON_IsArray(states)) {
        cJSON *plane = NULL;

        int plane_count = cJSON_GetArraySize(states);
        DrawText(TextFormat("Total Planes: %d", plane_count),10, 10, 20, GREEN);

        cJSON_ArrayForEach(plane, states) {
            cJSON *call_sign = cJSON_GetArrayItem(plane, 1);
            cJSON *longitude = cJSON_GetArrayItem(plane, 5);
            cJSON *latitude = cJSON_GetArrayItem(plane, 6);
            cJSON *baro_alt_m = cJSON_GetArrayItem(plane, 7);

            if (cJSON_IsString(call_sign) && cJSON_IsNumber(longitude) && cJSON_IsNumber(latitude)) {
                float lon = (float)longitude->valuedouble;
                float lat = (float)latitude->valuedouble;
                float alt_ft = (float)baro_alt_m->valuedouble * MT_TO_FT;

                Vector2 pos = MapGPSToRadar(lat, lon, LA_MIN, LA_MAX, LO_MIN, LO_MAX);

                DrawCircleV(pos, 4.0f, GREEN);
                DrawText(call_sign->valuestring, (int)pos.x + 6, (int)pos.y - 4, 10, LIGHTGRAY);
            }
        }
    }
    cJSON_Delete(root);
}

void DrawAirport(const char* airport_tag) {
    int text_size = MeasureText(airport_tag, 20);
    DrawText(airport_tag, WINDOW_WIDTH / 2 - text_size / 2, WINDOW_HEIGHT / 2 + 20, 20, GREEN);

}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Plane Radar");
    if (!IsWindowReady()) return 1;
    SetTargetFPS(0);

    float angle = 0.0f;

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
        DrawAirport("LHR");

        EndDrawing();
    }

    CloseWindow();
    return 0;
}