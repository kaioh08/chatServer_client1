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
#include <construct_packet.h>

#define CURRENT_VERSION 1

#define VERSION_BITS 4
#define TYPE_BITS 4
#define OBJECT_BITS 8
#define SIZE_BITS 16
#define BASE_HEADER_BYTES 4

#define VERSION_SHIFT 28
#define TYPE_SHIFT 24
#define OBJECT_SHIFT 16

#define DELIMITER "\3"

void create_user_dispatch(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, const char *login_token, const char *display_name, const char *password)
{
    size_t size_of_body;

//    packet = dc_malloc(env, err, sizeof(struct base_packet));
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

    char *body = dc_malloc(env, err, size_of_body+1);
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    strcpy(body, login_token);
    strcat(body, DELIMITER);
    strcat(body, display_name);
    strcat(body, DELIMITER);
    strcat(body, password);
    strcat(body, DELIMITER);

    //duplicate body into packet->body
    packet->body = (uint8_t*) strdup(body);

    free(body);
}

void create_channel_dispatch(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, const char *channel_name, const char *user_display_name, const bool is_public)
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
    body = dc_malloc(env, err, size_of_body+1);
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    strcpy(body, channel_name);
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

uint8_t *serialize_packet(const struct dc_env *env, struct dc_error *err, const struct base_packet *packet, size_t *packet_size)
{
    size_t current_pos;
    uint16_t temp;

    *packet_size = BASE_HEADER_BYTES;
    *packet_size += packet->size;
    temp = htons(packet->size);

    current_pos = 0;

    char *serialized_packet = dc_malloc(env, err, *packet_size);
    if(dc_error_has_error(err))
    {
        return 0;
    }

    uint32_t header = packet->version << VERSION_SHIFT |
            packet->type << TYPE_SHIFT |
            packet->object << OBJECT_SHIFT |
            temp;

    //TODO: fix this, for whatever reason after line 132, serialized_packet is still empty, and then an error occurs at line 134
    memcpy(serialized_packet, &header, sizeof(uint32_t));
    current_pos += sizeof(uint32_t);
    memcpy(serialized_packet+current_pos, packet->body, packet->size);

    return (uint8_t *) serialized_packet;
}

void deserialize_header(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, uint8_t *received_packet)
{
    packet = dc_malloc(env, err, sizeof(struct base_packet));
    uint16_t size_size;
    uint32_t header;

    memcpy(&header, &received_packet, BASE_HEADER_BYTES);
    packet->version = header >> VERSION_SHIFT;
    packet->type = header >> TYPE_SHIFT;
    packet->object = header >> OBJECT_SHIFT;
    size_size = header & 0xFFFF;
    packet->size = ntohs(size_size);
}

// send packet over TCP connection
int send_packet(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, char* server_ip, int server_port) {
    uint8_t *serialized_packet;
    uint8_t *received_packet;
    size_t packet_size;
    ssize_t wbytes;
    ssize_t rbytes;
    struct base_packet *deserialized_packet = NULL;

    int sock = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sock == -1) {
        perror("Error: failed to create socket");
        return -1;
    }

    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port);


    received_packet = NULL;
    serialized_packet = serialize_packet(env, err, packet, &packet_size);
    printf("serialized_packet: %s\n", serialized_packet); //TODO: why is serialized_packet empty?????

    if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0) { // Connect to server
        perror("Error: connection failed");
        return -1;
    }

    printf("Connected to server\n");

    wbytes = write_fully(sock, serialized_packet, packet_size);
    if((size_t) wbytes != packet_size)
    {
        perror("wbytes != packet_size\n");
    }

    rbytes = read(sock, received_packet, BASE_HEADER_BYTES);
    if(rbytes != BASE_HEADER_BYTES)
    {
        perror("rbytes != header bytes\n");
    }
    deserialize_header(env, err, deserialized_packet, received_packet);
    if(deserialized_packet == NULL)
    {
        perror("deserialized_packet still NULL\n");
    }

    rbytes = read(sock, received_packet, deserialized_packet->size);
    if(rbytes != deserialized_packet->size)
    {
        perror("rbytes != header bytes\n");
    }
    deserialized_packet->body = (uint8_t*) dc_strdup(env, err, (char *) received_packet);

    free(received_packet);
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

