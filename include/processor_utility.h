#ifndef PROCESS_SERVER_PROCESSOR_UTILITY_H
#define PROCESS_SERVER_PROCESSOR_UTILITY_H

#include <dc_env/env.h>
#include <dc_error/error.h>
#include <arpa/inet.h>
#include <dc_util/networking.h>
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
#include <poll.h>
#include <dc_util/io.h>

#define DEFAULT_SIZE 1024
#define DEFAULT_VERSION 0x1

enum Type {
    CREATE = 0x1,
    READ = 0x2,
    UPDATE = 0x3,
    DESTROY = 0x4,
    PINGUSER = 0x9
};

enum Object {
    USER = 0x01,
    CHANNEL = 0x02,
    MESSAGE = 0x03,
    AUTH = 0x04
};

struct binary_header_field {
    uint32_t version : 4; // 4 bit version number
    uint32_t type : 4; // 4 bit type number
    uint8_t object; // 8 bit object type
    uint16_t body_size; // 16 bit body size
};

struct arg {
    struct dc_env *env;
    struct dc_error *error;
    int socket_fd;
    FILE * debug_log_file;
    char *response_buffer;
    struct binary_header_field *b_header;
};

struct request{
    char * type;
    char * obj;
    char * data;
};

struct read_handler_args{
    struct dc_env *env;
    struct dc_error *err;
    char *response_buffer;
    struct binary_header_field *b_header;
    int socket_fd;
};

typedef void (*message_handler)(void *arg);

void *read_message_handler(void *arg);
void response_handler_wrapper(struct dc_env *env, struct dc_error *err, struct arg *options, struct binary_header_field *b_header, char *body);

void deserialize_header(struct dc_env *env, struct dc_error *err, int fd, struct binary_header_field *b_header, uint32_t value);
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

#endif //PROCESS_SERVER_PROCESSOR_UTILITY_H
