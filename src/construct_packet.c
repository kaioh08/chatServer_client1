#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// maximum length for named and password
#define MAX_NAMED_LENGTH 20
#define MAX_PASSWORD_LENGTH 6

struct Packet {               // packet structure for registering
    char named[MAX_NAMED_LENGTH + 1];
    char display_name[MAX_NAMED_LENGTH + 1];
    char password[MAX_PASSWORD_LENGTH + 1];
};

void create_packet(char named, char* display_name, char* password) {
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
    snprintf(packet.named, sizeof(packet.named), "%d", named);
    strncpy(packet.display_name, display_name, MAX_NAMED_LENGTH);
    packet.display_name[MAX_NAMED_LENGTH] = '\0';
    strncpy(packet.password, password, MAX_PASSWORD_LENGTH);
    packet.password[MAX_PASSWORD_LENGTH] = '\0';

    // send packet over TCP connection
    // TODO: implement TCP connection and send packet code
}
