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
#define BASE_PACKET_SIZE 32

#define TYPE_CREATE 1
#define TYPE_READ 2
#define TYPE_UPDATE 3
#define TYPE_DESTROY 4

#define OBJECT_USER 1
#define OBJECT_CHANNEL 2
#define OBJECT_MESSAGE 3
#define OBJECT_AUTH 4

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
};

struct register_packet {
    struct base_packet *base_packet;
    char *body;
};

void create_packet(char* named, char* display_name, char* password);
void create_register_packet(struct dc_env *env, struct dc_error *err, struct register_packet *packet, char *login_token, char *display_name, char *password);
size_t serialize_packet(struct dc_env *env, struct dc_error *err, void *packet, uint8_t *serialized_packet, size_t *size);
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

void create_register_packet(struct dc_env *env, struct dc_error *err, struct register_packet *packet, char *login_token, char *display_name, char *password)
{
    packet = dc_malloc(env, err, sizeof(struct register_packet));
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    packet->base_packet = dc_malloc(env, err, sizeof(struct base_packet));
    if(dc_error_has_error(err))
    {
        fprintf(stderr, "ERROR: %s \n", dc_error_get_message(err)); //NOLINT(cert-err33-c)
    }
    packet->base_packet->version = CURRENT_VERSION;
    packet->base_packet->type = TYPE_CREATE;
    packet->base_packet->object = OBJECT_USER;

    //TODO: append the login_token, display_name, and password tgt to form the body
    //TODO: copy size of body to packet->base_packet->size
}

size_t serialize_packet(struct dc_env *env, struct dc_error *err, void *packet, uint8_t *serialized_packet, size_t *size)
{
    size_t packet_size;
    packet_size = 0;

    dc_realloc(env, err, serialized_packet, BASE_PACKET_SIZE);
    //TODO: serialize the packet

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
