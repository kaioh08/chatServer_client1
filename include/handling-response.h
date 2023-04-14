#ifndef PROCESS_SERVER_HANDLING_RESPONSE_H
#define PROCESS_SERVER_HANDLING_RESPONSE_H

#include "processor_utility.h"

void handle_server_request(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_ping_user(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_ping_channel(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

void handle_server_create(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_read(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_update(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_delete(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

void handle_create_user_response(struct server_options *options, char *body);
void handle_create_channel_response(struct server_options *options, char *body);
void handle_create_message_response(struct server_options *options, char *body);
void handle_create_auth_response(struct server_options *options, char *body);

void handle_read_user_response(struct server_options *options, char *body);
void handle_read_channel_response(struct server_options *options, char *body);
void handle_read_message_response(struct server_options *options, char *body);
void handle_read_auth_response(struct server_options *options, char *body);


#endif //PROCESS_SERVER_HANDLING_RESPONSE_H
