//
// Created by Harry Skerritt on 05/08/2026.
//
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include "api.h"
#include "tokens.h"

const char* TOKEN_URL = "https://auth.opensky-network.org/auth/realms/opensky-network/protocol/openid-connect/token";
const int TOKEN_REFRESH_MARGIN = 30; // 30s before expiry
Token current_token = { .expiry_time = 0 };

char CLIENT_ID[128] = {0};
char CLIENT_SECRET[128] = {0};
float custom_lat = 0;
float custom_lon = 0;
char custom_name[64] = "My Location";

// Config
bool GetConfigPath(char *out_path, size_t max_len) {
    const char *home = getenv("HOME");
    if (home) {
        snprintf(out_path, max_len, "%s/flight_radar.config", home);
        return true;
    }
    return false;
}

bool LoadConfig(void) {
    char path[512];
    GetConfigPath(path, sizeof(path));

    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file %s\n", path);
        return false;
    }

    char buffer[256];

    // Client ID
    if (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        strncpy(CLIENT_ID, buffer, sizeof(CLIENT_ID) - 1);
    }

    // Client Secret
    if (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        strncpy(CLIENT_SECRET, buffer, sizeof(CLIENT_SECRET) - 1);
    }

    // Custom Lat & Lon
    if (fgets(buffer, sizeof(buffer), file)) {
        custom_lat = (float)atof(buffer);
    }
    if (fgets(buffer, sizeof(buffer), file)) {
        custom_lon = (float)atof(buffer);
    }

    // Custom Name
    if (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        strncpy(custom_name, buffer, sizeof(custom_name) - 1);
    }

    fclose(file);
    if (strlen(CLIENT_ID) == 0 || strlen(CLIENT_SECRET) == 0) {
        fprintf(stderr, "Credentials file is empty or malformed\n");
        return false;
    }

    return true;
}

bool SaveConfig(const char *client_id, const char *client_secret, float lat, float lon, const char *name) {
    char path[512];
    GetConfigPath(path, sizeof(path));

    FILE *file = fopen(path, "w");
    if (!file) {
        fprintf(stderr, "Failed to open config for writing\n");
        return false;
    }

    fprintf(file, "%s\n%s\n%.4f\n%.4f\n%s\n", client_id, client_secret, lat, lon, name);
    fclose(file);

    strncpy(CLIENT_ID, client_id, sizeof(CLIENT_ID));
    strncpy(CLIENT_SECRET, client_secret, sizeof(CLIENT_SECRET));
    custom_lat = lat;
    custom_lon = lon;
    strncpy(custom_name, name, sizeof(custom_name));
    return true;
}

// Tokens
Token GetToken(void) {
    if (strlen(current_token.token) > 0 && time(NULL) < (current_token.expiry_time - TOKEN_REFRESH_MARGIN)) {
        return current_token;
    } else {
        return RefreshToken();
    }
}

Token RefreshToken(void) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    char post_data[512];
    snprintf(post_data, sizeof(post_data),
             "grant_type=client_credentials&client_id=%s&client_secret=%s",
             CLIENT_ID, CLIENT_SECRET);

    char *res_json = PostEndpoint(TOKEN_URL, headers, post_data);
    curl_slist_free_all(headers);

    if (!res_json) {
        printf("Authentication Failed: No Response\n");
        const Token empty = { 0 };
        return empty;
    }

    cJSON *root = cJSON_Parse(res_json);
    free(res_json);

    if (!root) {
        printf("Authentication Failed: Parse error\n");
        const Token empty = {0};
        return empty;
    }

    const cJSON *token_item = cJSON_GetObjectItemCaseSensitive(root, "access_token");
    const cJSON *expire_item = cJSON_GetObjectItemCaseSensitive(root, "expires_in");

    if (cJSON_IsString(token_item) && token_item->valuestring != NULL) {
        strncpy(current_token.token, token_item->valuestring, sizeof(current_token.token) - 1);
    }

    const int expires_in = (cJSON_IsNumber(expire_item) ? expire_item->valueint : 1800);
    current_token.expiry_time = time(NULL) + (time_t)expires_in;

    cJSON_Delete(root);
    return current_token;
}

struct curl_slist* BuildAuthHeaders(void) {
    struct curl_slist *headers = NULL;
    char auth_header[2048];

    const Token token = GetToken();

    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token.token);
    headers = curl_slist_append(headers, auth_header);

    return headers;
}