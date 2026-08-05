//
// Created by Harry Skerritt on 05/08/2026.
//

#ifndef API_H
#define API_H
#include <curl/curl.h>

void UpdatePlaneData(void);
char* GetEndpoint(const char *url, struct curl_slist *headers);
char* PostEndpoint(const char *url, struct curl_slist *headers, const char *post_fields);

#endif //API_H
