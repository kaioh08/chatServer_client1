#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dc_error/error.h>
#include <dc_env/env.h>
#include <dc_c/dc_stdlib.h>
#include <dc_posix/dc_string.h>

#define CURRENT_VERSION 1

#define VERSION_BITS 4
#define TYPE_BITS 4
#define OBJECT_BITS 8
#define SIZE_BITS 16
#define BASE_HEADER_BYTES 4

#define VERSION_SHIFT 28
#define TYPE_SHIFT 24
#define OBJECT_SHIFT 16

#define RESPONSE_SIZE 1024

#define DELIMITER "\3"

enum types {
    TYPE_CREATE = 1,
    TYPE_READ,
    TYPE_UPDATE,
    TYPE_DESTROY
};

enum objects {
    OBJECT_USER = 1,
    OBJECT_CHANNEL,
    OBJECT_MESSAGE,
    OBJECT_AUTH
};

struct base_packet {
    uint8_t version;
    uint8_t type;
    uint8_t object;
    uint16_t size;
    uint8_t *body;
};

void create_user_dispatch(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, char *login_token, char *display_name, char *password);
void create_channel_dispatch(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, char *channel_name, char *user_display_name, bool is_public);
size_t serialize_packet(const struct dc_env *env, struct dc_error *err, const struct base_packet *packet, uint8_t *serialized_packet);
void deserialize_packet(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, uint8_t *received_packet, ssize_t rbytes);
int send_packet(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, char* server_ip, int server_port);
ssize_t write_fully(int fd, const void *buffer, size_t len);
ssize_t read_fully(int fd, void *buffer, size_t len);


void create_user_dispatch(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, char *login_token, char *display_name, char *password)
{
    size_t size_of_body;
    char *body;

    packet = dc_malloc(env, err, sizeof(struct base_packet));
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    packet->version = CURRENT_VERSION;
    packet->type = TYPE_CREATE;
    packet->object = OBJECT_USER;

    //sum up the size of each field + the \3 character
    size_of_body = strlen(login_token)+1;
    size_of_body += strlen(display_name)+1;
    size_of_body += strlen(password)+1;
    packet->size = size_of_body;

    //copy each field to the body
    body = dc_malloc(env, err, size_of_body*sizeof(char));
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    strcat(body, login_token);
    strcat(body, DELIMITER);
    strcat(body, display_name);
    strcat(body, DELIMITER);
    strcat(body, password);
    strcat(body, DELIMITER);

    //duplicate body into packet->body
    packet->body = (uint8_t*) strdup(body);

    free(body);
}

void create_channel_dispatch(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, char *channel_name, char *user_display_name, bool is_public)
{
    size_t size_of_body;
    char *body;

    packet = dc_malloc(env, err, sizeof(struct base_packet));
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    packet->version = CURRENT_VERSION;
    packet->type = TYPE_CREATE;
    packet->object = OBJECT_CHANNEL;

    size_of_body = strlen(channel_name)+1;
    size_of_body += strlen(user_display_name)+1;
    size_of_body += 1+1;
    packet->size = size_of_body;

    //copy each field to the body
    body = dc_malloc(env, err, size_of_body*sizeof(char));
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    strcat(body, channel_name);
    strcat(body, DELIMITER);
    strcat(body, user_display_name);
    strcat(body, DELIMITER);
    if(is_public)
    {
        strcat(body, "1");
    }
    else
    {
        strcat(body, "0");
    }
    strcat(body, DELIMITER);

    //duplicate body into packet->body
    packet->body = (uint8_t*) strdup(body);

    free(body);
}

size_t serialize_packet(const struct dc_env *env, struct dc_error *err, const struct base_packet *packet, uint8_t *serialized_packet)
{
    size_t packet_size;
    size_t current_pos;
    uint16_t temp;

    packet_size = BASE_HEADER_BYTES;
    packet_size += packet->size;
    temp = htons(packet->size);

    current_pos = 0;

    dc_realloc(env, err, serialized_packet, packet_size);
    if(dc_error_has_error(err))
    {
        return 0;
    }

    uint32_t header = packet->version << VERSION_SHIFT |
            packet->type << TYPE_SHIFT |
            packet->object << OBJECT_SHIFT |
            temp;
    memcpy(&serialized_packet[current_pos], &header, BASE_HEADER_BYTES);
    current_pos += BASE_HEADER_BYTES;

    memcpy(&serialized_packet[current_pos], &packet->body, packet->size);

    return packet_size;
}

void deserialize_packet(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, uint8_t *received_packet, ssize_t rbytes)
{
    size_t current_pos = 0;
    packet = dc_malloc(env, err, rbytes);
    uint16_t size_size;
    uint32_t header;

}

// send packet over TCP connection
int send_packet(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, char* server_ip, int server_port) {
    uint8_t *serialized_packet;
    uint8_t *received_packet;
    size_t packet_size;
    ssize_t wbytes;
    ssize_t rbytes;
    struct base_packet *deserialized_packet;

    int sock = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sock == -1) {
        perror("Error: failed to create socket");
        return -1;
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);

    if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0) { // Connect to server
        perror("Error: connection failed");
        return -1;
    }

    serialized_packet = NULL;
    received_packet = NULL;
    packet_size = serialize_packet(env, err, packet, serialized_packet);

    wbytes = write_fully(sock, serialized_packet, packet_size);
    if((size_t) wbytes != packet_size)
    {
        perror("wbytes != packet_size\n");
    }

    //TODO: read into a 32bit integer, do the shifts, read the next however many bytes specified by size

    close(sock);
    return 0;
}


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

