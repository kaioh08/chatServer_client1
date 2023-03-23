#ifndef PROCESS_SERVER_PROCESSOR_UTILITY_H
#define PROCESS_SERVER_PROCESSOR_UTILITY_H

#include <dc_env/env.h>
#include <dc_error/error.h>
#include <arpa/inet.h>

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

//changed it to uint32_t to see if the network order is correct
struct binary_header {
//    unsigned int version : 4; // 4 bit for version number
//    unsigned int type : 4; // 4 bit for type number
    uint32_t version : 4; // 4 bit for version number
    uint32_t type : 4; // 4 bit for type number
    uint8_t object; // 8 bit for object type
    uint16_t body_size; // 16 bit for body size
};
struct binary_header * deserialize_header(uint32_t value);
void display_header(struct binary_header * header, const char * data);
void serialize_header(struct dc_env *env, struct dc_error *err, struct binary_header * header, int fd, const char * body);
/**
 * Create
 */
void send_create_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_create_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);

/**
 * Read
 */
void send_read_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_read_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_read_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);

/**
 * Update
 */
void send_update_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_update_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_update_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_update_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);

/**
 * Delete
 */
void send_delete_user(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_delete_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_delete_message(struct dc_env *env, struct dc_error *err, int fd, const char * body);
void send_delete_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body);

#endif //PROCESS_SERVER_PROCESSOR_UTILITY_H
