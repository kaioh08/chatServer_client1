#include "../include/processor_utility.h"
#include <arpa/inet.h>
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <dc_env/env.h>
#include <dc_error/error.h>
#include <dc_posix/dc_unistd.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>

#define SERVER_PORT 5000
#define MAX_SIZE 1024

int main(int argc, char *argv[])
{
    struct dc_env *env;
    struct dc_error *err;
    int socket_fd;
    struct sockaddr_in server_addr;
    char buffer[MAX_SIZE];
    bool run_client = true;

    if (argc < 2)
    {
        fprintf(stderr, "Server IP: %s <server_ip>\n", argv[0]);
        run_client = false;
    }

    err = dc_error_create(true);
    env = dc_env_create(err, true, NULL);

    char buffer2[1024];
    ssize_t num_read = read(STDIN_FILENO, buffer2, sizeof(buffer2));
    if (num_read == -1) {
        perror("read failed");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Child process received: %.*s", (int)num_read, buffer2);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        run_client = false;
    }

    dc_memset(env, &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("INET_PTON failed");
        run_client = false;
    }

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect failed");
        run_client = false;
    }

    if (run_client) {
        fprintf(stderr, "Connected to server.\n");

        while(fgets(buffer, MAX_SIZE, stdin) != NULL)
        {
            ssize_t n1 = send(socket_fd, buffer, dc_strlen(env, buffer), 0);

            if (n1 < 0)
            {
                perror("send");
                return EXIT_FAILURE;
            }

            fprintf(stderr, "Written to server\n");
            write(STDOUT_FILENO, buffer, n1);

            uint32_t header;
            char body[MAX_SIZE];
            ssize_t n;

            // receive header from server
            n = read(socket_fd, &header, sizeof(header));
            if (n < 0) {
                perror("error");
                exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
            }

            struct binary_header * binaryHeaderField = deserialize_header(header);

            // print deserialized header
            fprintf(stderr, "Version: %d\n", binaryHeaderField->version);
            fprintf(stderr, "Type: %d\n", binaryHeaderField->type);
            fprintf(stderr, "Object: %d\n", binaryHeaderField->object);
            fprintf(stderr, "Body Size: %d\n", binaryHeaderField->body_size);

            // Read body and clear buffer
            read(socket_fd, &body, MAX_SIZE);
            body[(binaryHeaderField->body_size)] = '\0';
            fprintf(stderr, "Body: %s\n", body);
        }
        fprintf(stderr, "Client Disconnected.\n");
    }

    free(env);
    free(err);
    close(socket_fd);

    return EXIT_SUCCESS;
}