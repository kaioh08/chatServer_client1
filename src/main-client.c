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

struct binary_header * deserialize_header(uint32_t value) {
    struct binary_header * header;
    header = malloc(sizeof(struct binary_header));
    header->version = (value >> 28) & 0x0F;
    header->type = (value >> 24) & 0x0F;
    header->object = (value >> 16) & 0xFF;
    header->body_size = value & 0xFFFF;
    header->body_size = ntohs(header->body_size);

    return header;
}

void display_header(struct binary_header * header, const char * data)
{
    printf("DATA PACKET\n");
    printf("Packet version: %d\n", header->version);
    printf("Packet type:  %d\n", header->type);
    printf("Packet object type: %d\n", header->object);
    printf("Packet body size: %d\n", header->body_size);
    printf("Packet body: %s\n", data);
}

void serialize_header(struct dc_env *env, struct dc_error *err, struct binary_header * header, int fd,
                      const char * body)
{
    char data[DEFAULT_SIZE];

//    header->body_size = htonl(header->body_size); // Convert to network byte order.

    // Create the packet
    uint32_t packet = ((header->version & 0xF) << 28) | ((header->type & 0xF) << 24) | ((header->object & 0xFF) << 16) | (header->body_size & 0xFFFF);  // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    packet = htonl(packet); // Convert to network byte order.
//     packet = ntohl(packet);
    // Copy the packet into buffer
    dc_memcpy(env, data, &packet, sizeof(uint32_t));

    // Add the body to buffer
    dc_memcpy(env, data + sizeof(uint32_t), body, dc_strlen(env, body));
    dc_write(env, err, fd, &data, (sizeof(uint32_t) + dc_strlen(env, body)));
}


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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
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
    struct binary_header header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    // Display and send the header
    display_header(&header, body);
    serialize_header(env, err, &header, fd, body);
}
