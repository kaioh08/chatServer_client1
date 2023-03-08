#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int send_signup_request(char* username, char* password) {
    int sockfd;
    ssize_t n;
    struct sockaddr_in serv_addr;
    char buffer[1024];

// Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }

// Get server address from user input
    char server_address[50];
    printf("Enter server address: ");
    scanf("%s", server_address);

// Set up the server address struct
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, server_address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid server address");
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        return -1;
    }

    // Construct the request packet
    sprintf(buffer, "SIGNUP %s %s", username, password);

    // Send the request packet to the server
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) {
        perror("ERROR writing to socket");
        return -1;
    }

    // Receive the response packet from the server
    n = read(sockfd, buffer, 1024);
    if (n < 0) {
        perror("ERROR reading from socket");
        return -1;
    }

    // Parse the response packet and check if it was successful
    if (strcmp(buffer, "SUCCESS") == 0) {
        printf("Signup successful!\n");
        return 0;
    } else {
        printf("Signup failed: %s\n", buffer);
        return -1;
    }

    close(sockfd);
}

int main() {
    char* username = malloc(21 * sizeof(char));
    char* password = malloc(7 * sizeof(char));
    printf("Enter username: ");
    scanf("%20s", username);
    printf("Enter password: ");
    scanf("%6s", password);

    int result = send_signup_request(username, password);

    free(username);
    free(password);

    return result;
}
