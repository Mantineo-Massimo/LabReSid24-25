#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include "http_parser.h"
#include "resource_manager.h"

void handle_client(int client_socket, resource_manager_t* rm);

#endif
