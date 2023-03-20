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

enum Type {
    CREATE = 0x1,
    READ = 0x2,
    UPDATE = 0x3,
    DESTROY = 0x4,
    PING = 0x8
};

enum Object {
    USER = 0x01,
    CHANNEL = 0x02,
    MESSAGE = 0x03,
    AUTH = 0x04
};

struct binary_header_field {
    unsigned int version : 4; // 4 bit version number
    unsigned int type : 4; // 4 bit type number
    uint8_t object; // 8 bit object type
    uint16_t body_size; // 16 bit body size
};


ssize_t write_fully(int fd, const void *buffer, size_t len);
ssize_t read_fully(int fd, void *buffer, size_t len);
void display_header(struct binary_header_field * header, const char * data);
struct binary_header_field * deserialize_header(uint32_t value);
void serialize_header(struct dc_env *env, struct dc_error *err, struct binary_header_field * header, int fd,
                      const char * body);

void send_create_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_read_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_read_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_read_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_update_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_update_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_update_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_update_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_delete_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_delete_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_delete_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_delete_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);



int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

    if (argc < 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return EXIT_FAILURE;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("listening_socket");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        return EXIT_FAILURE;
    }

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        return EXIT_FAILURE;
    }

    printf("Connected to server.\n");

    ssize_t n_user;

    while((n_user = read(STDIN_FILENO, buffer, BUF_SIZE)) > 0)
    {
        write_fully(sockfd, buffer, n_user);
        ssize_t n_read = read_fully(sockfd, buffer, n_user);
        write_fully(STDOUT_FILENO, buffer, n_read);
    }

    close(sockfd);

    return EXIT_SUCCESS;
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


// send login (create auth) request
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

//Send log out(delete auth)  user request
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