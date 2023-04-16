#ifndef PROCESS_SERVER_HANDLING_RESPONSE_H
#define PROCESS_SERVER_HANDLING_RESPONSE_H

#include "processor_utility.h"
#include "global_vars.h"
#include "gui.h"

enum response_code
{
    ALL_GOOD = 200,
    CREATED = 201,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    NOT_UNIQUE = 409,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
};

void clear_debug_file_buffer(FILE * debug_log_file);

void handle_server_request(struct arg * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_ping_user(struct arg * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_ping_channel(struct arg * options, struct binary_header_field * binaryHeaderField, char * body);

void handle_server_create(struct arg * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_read(struct arg * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_update(struct arg * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_delete(struct arg * options, struct binary_header_field * binaryHeaderField, char * body);

int handle_create_user_response(struct arg *options, char *body);
int handle_create_channel_response(struct arg *options, char *body);
int handle_create_message_response(struct arg *options, char *body);
int handle_create_auth_response(struct arg *options, char *body);

void handle_read_user_response(struct arg *options, char *body);
void handle_read_channel_response(struct arg *options, char *body);
void handle_read_message_response(struct arg *options, char *body);
void handle_read_auth_response(struct arg *options, char *body);


#endif //PROCESS_SERVER_HANDLING_RESPONSE_H
