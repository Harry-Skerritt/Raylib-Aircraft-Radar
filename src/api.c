//
// Created by Harry Skerritt on 05/08/2026.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "config.h"
#include "api.h"

#include "tokens.h"

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

char* GetEndpoint(const char *url, struct curl_slist *headers) {
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

    if (headers != NULL) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    const CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    printf("HTTP Status Code: %ld\n", http_code);

    if (res != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        chunk.memory = NULL;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return chunk.memory;
}

char* PostEndpoint(const char *url, struct curl_slist *headers, const char *post_fields) {
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
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "C-Radar-App/1.0");

    if (headers != NULL) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

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

void UpdatePlaneData() {
    fetch_timer += GetFrameTime();

    if (fetch_timer < 10.0f && cached_json != NULL) {
        return;
    }
    fetch_timer = 0.0f;


    char url[512];
    snprintf(url, sizeof(url),
             "https://opensky-network.org/api/states/all?lamin=%.4f&lamax=%.4f&lomin=%.4f&lomax=%.4f",
             la_min, la_max, lo_min, lo_max);

    printf("HTTP URL: %s\n", url);

    struct curl_slist *headers = BuildAuthHeaders();
    char* new_json = GetEndpoint(url, headers);

    if (headers) {
        curl_slist_free_all(headers);
    }

    if (!new_json) {
        printf("Failed to fetch airspace data.\n");
        return;
    }

    if (cached_json) {
        free(cached_json);
    }
    cached_json = new_json;
}