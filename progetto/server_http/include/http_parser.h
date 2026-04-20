#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#define MAX_HTTP_SIZE 8192
#define MAX_METHOD_LEN 8
#define MAX_URI_LEN 256
#define MAX_BODY_LEN 4096

typedef struct {
    char method[MAX_METHOD_LEN];
    char uri[MAX_URI_LEN];
    char body[MAX_BODY_LEN];
    int body_len;
} http_request_t;

int parse_http_request(const char* raw_request, http_request_t* request);

#endif
