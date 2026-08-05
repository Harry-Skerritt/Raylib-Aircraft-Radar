//
// Created by Harry Skerritt on 05/08/2026.
//

#ifndef TOKENS_H
#define TOKENS_H

#include <time.h>
#include <curl/curl.h>
#include <stdbool.h>

typedef struct {
    char token[2048];
    time_t expiry_time;
} Token;

// Const Vars
extern const char* TOKEN_URL;
extern char CLIENT_ID[128];
extern char CLIENT_SECRET[128];
extern const int TOKEN_REFRESH_MARGIN;
extern Token current_token;

bool LoadCredentials(const char* filename);
Token GetToken(void);
Token RefreshToken(void);
struct curl_slist* BuildAuthHeaders(void);


#endif //TOKENS_H
