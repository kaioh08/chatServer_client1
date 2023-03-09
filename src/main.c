#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dc_env/env.h>
#include "construct_packet.h"

#define SERVER_PORT 5000
#define BUF_SIZE 256

int main(int argc, char *argv[])
{
    dc_env_tracer tracer;
    struct dc_error * err;
    struct dc_env * env;

    // Set the tracer to trace through the function calls
//    tracer = dc_env_default_tracer; // Trace through function calls
    tracer = NULL; // Don't trace through function calls

    err = dc_error_create(false); // Create error struct
    env = dc_env_create(err, false, tracer); // Create environment struct

    dc_env_set_tracer(env, *tracer); // Set tracer

    //fork
    //child handles (thread pool? of) requests and responses
    //parent runs ui

    struct base_packet test;
    memset(&test, 0, sizeof(struct base_packet));
    create_user_dispatch(env, err, &test, "login_token", "display_name", "password");
    send_packet(env, err, &test, argv[1], SERVER_PORT);

    return EXIT_SUCCESS;
}
