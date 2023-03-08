#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dc_error/error.h>
#include <dc_env/env.h>
#include <dc_c/dc_stdlib.h>

// maximum length for named and password
#define MAX_NAMED_LENGTH 20
#define MAX_PASSWORD_LENGTH 6

#define CURRENT_VERSION 1

#define VERSION_SIZE 4
#define TYPE_SIZE 4
#define OBJECT_SIZE 8
#define SIZE_SIZE 16
#define BASE_HEADER_SIZE 32

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

struct Packet {               // packet structure for registering
    char named[MAX_NAMED_LENGTH + 1];
    char display_name[MAX_NAMED_LENGTH + 1];
    char password[MAX_PASSWORD_LENGTH + 1];
};

struct base_packet {
    uint8_t version;
    uint8_t type;
    uint8_t object;
    uint16_t size;
    uint8_t *body;
};

void create_packet(char* named, char* display_name, char* password);
void create_user_dispatch(struct dc_env *env, struct dc_error *err, struct base_packet *packet, char *login_token, char *display_name, char *password, size_t *size_of_body);
size_t serialize_packet(struct dc_env *env, struct dc_error *err, const struct base_packet *packet, uint8_t *serialized_packet, const size_t *size_of_body);
int send_packet(char* packet, char* server_ip, int server_port);

void create_packet(char* named, char* display_name, char* password) {
    struct Packet packet;

    // check named length
    if (strlen(named) > MAX_NAMED_LENGTH) {
        printf("Error: Name is longer than maximum length of %d characters.\n", MAX_NAMED_LENGTH);
        return;
    }

    // check password length
    if (strlen(password) > MAX_PASSWORD_LENGTH) {
        printf("Error: Password is longer than maximum length of %d characters.\n", MAX_PASSWORD_LENGTH);
        return;
    }

    // copy arguments to packet fields
    snprintf(packet.named, sizeof(packet.named), "%s", named);
    strncpy(packet.display_name, display_name, MAX_NAMED_LENGTH);
    packet.display_name[MAX_NAMED_LENGTH] = '\0';
    strncpy(packet.password, password, MAX_PASSWORD_LENGTH);
    packet.password[MAX_PASSWORD_LENGTH] = '\0';
}

void create_user_dispatch(struct dc_env *env, struct dc_error *err, struct base_packet *packet, char *login_token, char *display_name, char *password, size_t *size_of_body)
{
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
    *size_of_body = strlen(login_token)+1;
    *size_of_body += strlen(display_name)+1;
    *size_of_body += strlen(password)+1;

    //copy each field to the body
    body = dc_malloc(env, err, (*size_of_body)*sizeof(char));
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    strcat(body, login_token);
    strcat(body, "\3");
    strcat(body, display_name);
    strcat(body, "\3");
    strcat(body, password);
    strcat(body, "\3");

    //duplicate body into packet->body
    packet->body = (uint8_t*) strdup(body);

    free(body);
}

size_t serialize_packet(struct dc_env *env, struct dc_error *err, const struct base_packet *packet, uint8_t *serialized_packet, const size_t *size_of_body)
{
    size_t packet_size;
    size_t current_pos;

    packet_size = BASE_HEADER_SIZE;
    packet_size += *size_of_body;

    current_pos = 0;

    dc_realloc(env, err, serialized_packet, packet_size);
    if(dc_error_has_error(err))
    {
        return 0;
    }
    //TODO: add packet->version to serialized_packet
    current_pos += VERSION_SIZE;
    //TODO: add packet->type to serialized_packet
    current_pos += TYPE_SIZE;
    //TODO: add packet->object to serialized_packet
    current_pos += OBJECT_SIZE;
    //TODO: add packet->size to serialized_packet
    current_pos += SIZE_SIZE;
    //TODO: add packet->body to serialized_packet

    return packet_size;
}

// send packet over TCP connection
int send_packet(char* packet, char* server_ip, int server_port) {
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

    if (send(sock, packet, strlen(packet), 0) < 0) { // Send packet
        perror("Error: failed to send packet");
        return -1;
    }
    close(sock);
    return 0;
}
