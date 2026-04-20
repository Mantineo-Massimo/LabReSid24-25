#include "../include/http_parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int parse_http_request(const char* raw_request, http_request_t* request) {
    memset(request, 0, sizeof(http_request_t));

    char first_line[MAX_URI_LEN + MAX_METHOD_LEN + 16];
    const char* line_end = strstr(raw_request, "\r\n");
    if (!line_end) return -1;

    size_t line_len = line_end - raw_request;
    if (line_len >= sizeof(first_line)) return -1;
    strncpy(first_line, raw_request, line_len);
    first_line[line_len] = '\0';

    if (sscanf(first_line, "%s %s", request->method, request->uri) != 2) {
        return -1;
    }

    const char* body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        request->body_len = strlen(body_start);
        if (request->body_len < MAX_BODY_LEN) {
            strncpy(request->body, body_start, MAX_BODY_LEN - 1);
        } else {
            strncpy(request->body, body_start, MAX_BODY_LEN - 1);
            request->body_len = MAX_BODY_LEN - 1;
        }
    }

    return 0;
}
