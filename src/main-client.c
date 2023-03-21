#include "../include/processor_utility.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <dc_env/env.h>
#include <dc_error/error.h>
#include <dc_posix/dc_unistd.h>
#include <netinet/in.h>


#define SERVER_PORT 4981
#define BUF_SIZE 256
#define DEFAULT_SIZE 1024
#define DEFAULT_VERSION 0x1


ssize_t write_fully(int fd, const void *buffer, size_t len);
ssize_t read_fully(int fd, void *buffer, size_t len);

ssize_t write_fully(int fd, const void *buffer, size_t len)
{
    ssize_t total_bytes_write = 0;

    while (total_bytes_write < (ssize_t)len)
    {
        ssize_t bytes_write = write(fd, (const uint8_t *)buffer + total_bytes_write, len - total_bytes_write);

        if(bytes_write == -1)
        {
            perror("write");
            exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
        }

        total_bytes_write += bytes_write;
    }

    return total_bytes_write;
}

ssize_t read_fully(int fd, void *buffer, size_t len)
{
    ssize_t total_bytes_read = 0;

    while (total_bytes_read < (ssize_t)len)
    {
        ssize_t bytes_read = read(fd, (uint8_t *)buffer + total_bytes_read, len - total_bytes_read);

        if(bytes_read == -1)
        {
            perror("write");
            exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
        }

        total_bytes_read += bytes_read;
    }

    return total_bytes_read;
}
void send_create_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

void send_create_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//send message
void send_create_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

void send_create_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//Send Read user request
void send_read_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//Send Read channel request
void send_read_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//Send Read message request
void send_read_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//Send Update Stuff
void send_update_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = UPDATE;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

void send_update_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = UPDATE;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

void send_update_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

void send_update_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//Send delete user request
void send_delete_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//Send delete channel request
void send_delete_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//Send delete message request
void send_delete_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}

//Send log out(delete auth) user request
void send_delete_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    // Create header
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}
