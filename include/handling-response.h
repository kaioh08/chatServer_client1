#ifndef PROCESS_SERVER_HANDLING_RESPONSE_H
#define PROCESS_SERVER_HANDLING_RESPONSE_H

#include "processor_utility.h"

void handle_server_request(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

#endif //PROCESS_SERVER_HANDLING_RESPONSE_H
