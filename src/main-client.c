#include "../include/processor_utility.h"
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

void *read_message_handler(void *arg)
{
    struct dc_env *env;
    struct dc_error *err;
    int fd;

    struct arg *args = (struct arg *) arg;
    env = args->env;
    err = args->error;
    fd = args->socket_fd;

    DC_TRACE(env);

    struct pollfd fds[1];
    int timeout;
    int ret;

    memset(fds, 0, sizeof(fds));
    timeout = 100;
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    while(true)
    {
//        printf("Anything new?\n");
        ret = poll(fds, 1, timeout);
        if (ret < 0) {
            perror("poll failed\n");
        }
        else if (ret == 0)
        {
//            printf("Didn't read anything in time\n");
            continue;
        }
        else
        {
            //when fd has stuff, read the first few bytes to get the header fields
            struct binary_header_field *b_header;
            uint32_t unprocessed_binary_header;
            ssize_t nread;
            nread = dc_read(env, err, fd, &unprocessed_binary_header, sizeof(uint32_t));
            if(nread != sizeof(uint32_t))
            {
                perror("read failed at reading binary header\n");
            }
            b_header = deserialize_header(unprocessed_binary_header);
            //read the dispatch after getting the binary header
            char *body;
            body = dc_malloc(env, err, b_header->body_size);
            nread = dc_read(env, err, fd, body, b_header->body_size);
            if(nread != b_header->body_size)
            {
                perror("read failed at reading body\n");
            }
            //parse through the dispatch, call the right response handler
            response_handler_wrapper(env, err, b_header, body);
        }
    }
}

void response_handler_wrapper(struct dc_env *env, struct dc_error *err, struct binary_header_field *b_header, char *body)
{
    switch(b_header->type)
    {
        case CREATE:
        {
            switch(b_header->object)
            {
                case USER:
                {
                    printf("call CREATE_USER response handler\n");
                    break;
                }
                case CHANNEL:
                {
                    printf("call CREATE_CHANNEL response handler\n");
                    break;
                }
                case MESSAGE:
                {
                    printf("call CREATE_MESSAGE response handler\n");
                    break;
                }
                case AUTH:
                {
                    printf("call CREATE_AUTH response handler\n");
                    break;
                }
                default:
                {
                    perror("bad object\n");
                }
            }
            break;
        }
        case READ:
        {
            switch(b_header->object)
            {
                case USER:
                {
                    printf("call READ_USER response handler\n");
                    break;
                }
                case CHANNEL:
                {
                    printf("call READ_CHANNEL response handler\n");
                    break;
                }
                case MESSAGE:
                {
                    printf("call READ_MESSAGE response handler\n");
                    break;
                }
                case AUTH:
                {
                    printf("call READ_AUTH response handler\n");
                    break;
                }
                default:
                {
                    perror("bad object\n");
                }
            }
            break;
        }
        case UPDATE:
        {            switch(b_header->object)
            {
                case USER:
                {
                    printf("call UPDATE_USER response handler\n");
                    break;
                }
                case CHANNEL:
                {
                    printf("call UPDATE_CHANNEL response handler\n");
                    break;
                }
                case MESSAGE:
                {
                    printf("call UPDATE_MESSAGE response handler\n");
                    break;
                }
                case AUTH:
                {
                    printf("call UPDATE_AUTH response handler\n");
                    break;
                }
                default:
                {
                    perror("bad object\n");
                }
            }
            break;
        }
        case DESTROY:
        {            switch(b_header->object)
            {
                case USER:
                {
                    printf("call DESTROY_USER response handler\n");
                    break;
                }
                case CHANNEL:
                {
                    printf("call DESTROY_CHANNEL response handler\n");
                    break;
                }
                case MESSAGE:
                {
                    printf("call DESTROY_MESSAGE response handler\n");
                    break;
                }
                case AUTH:
                {
                    printf("call DESTROY_AUTH response handler\n");
                    break;
                }
                default:
                {
                    perror("bad object\n");
                }
            }
            break;
        }
        default:
        {
            perror("bad type\n");
        }
    }
}

// create a struct called login_info and put the username and channel size, channel names in the struct at the
// moment of login, THEN  if the list of is changed, update the struct
struct login_info {
    char * username;
    unsigned int channel_size;
    char ** channel_names;
};

// to save the username and channel_size and channel_name after you login
static void save_username(struct dc_env *env, struct dc_error *err, struct login_info * login, const char * username) {
    login->username = dc_strncpy(env, err, username);
}

// add more channels after last channel names (assuming the channel name cannot be duplicated)
static void add_channel(struct dc_env *env, struct dc_error *err, struct login_info * login, const char * channel_name) {
    login->channel_names[login->channel_size] = dc_strncpy(env, err, channel_name);
    login->channel_size++;
}

// remove a channel from the list of channel names
static void remove_channel(struct dc_env *env, struct dc_error *err, struct login_info * login, const char * channel_name) {
    for (unsigned int i = 0; i < login->channel_size; i++) {
        if (dc_strcmp(env, login->channel_names[i], channel_name) == 0) {
            dc_free(env, login->channel_names[i]);
            login->channel_names[i] = login->channel_names[login->channel_size - 1];
            login->channel_size--;
            break;
        }
    }
}

// claer the username and channel_size and channel_name after you logout
// after you free the allocated memory, you need to set the pointer to NULL
static void clear_username(struct dc_env *env, struct dc_error *err, struct login_info * login) {
    char *username = (login->username);
    username = NULL;
    dc_free(env, username);
    for (unsigned int i = 0; i < login->channel_size; i++) {
        dc_free(env, login->channel_names[i]);
    }
    dc_free(env, login->channel_names);
    login->channel_size = 0;
}


struct binary_header_field * deserialize_header(struct dc_env *env, struct dc_error *err, int fd, uint32_t value) {
    struct binary_header_field * header;
    uint32_t header2;

    // Convert to network byte order
    value = ntohl(value);

    header = malloc(sizeof(struct binary_header_field));
    header->version = (value >> 28) & 0x0F; // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    header->type = (value >> 24) & 0x0F;    // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // read the remaining 3 bytes
    dc_read_fully(env, err, fd, &header2, 3);
    // Convert to network byte order
    header2 = ntohl(header2);
    header->object = (header2 >> 24) & 0xFF;  // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    header->body_size = (header2 >> 8) & 0xFFFF;     // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    return header;
}

void serialize_header(struct dc_env *env, struct dc_error *err, struct binary_header_field * header, int fd,
                      const char * body)
{
    char data[DEFAULT_SIZE];

    // Create the packet
    uint32_t packet = (((((uint32_t)header->version) & 0xF) << 28)) | ((((uint32_t)header->type) & 0xF) << 24) | // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                      ((((uint32_t)header->object) & 0xFF) << 16) | (((uint32_t)header->body_size) & 0xFFFF);  // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // Convert to network byte order.
    packet = htonl(packet);
    // Copy the packet into the data buffer
    dc_memcpy(env, data, &packet, sizeof(uint32_t));
    // Add the body to the data buffer
    dc_memcpy(env, data + sizeof(uint32_t), body, dc_strlen(env, body));

    dc_write(env, err, fd, &data, (sizeof(uint32_t) + dc_strlen(env, body)));
}

void clear_debug_buffer(FILE * debug_log_file)
{
    fflush(debug_log_file);
    setbuf(debug_log_file, NULL);
}

void send_create_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_create_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_create_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_create_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_read_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_read_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_read_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = READ;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_update_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = UPDATE;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_update_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = UPDATE;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_update_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_update_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = CREATE;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_delete_user(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = USER;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

void send_delete_channel(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);
    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = CHANNEL;
    header.body_size = dc_strlen(env, body);
    // Send the header
    serialize_header(env, err, &header, fd, body);
}

void send_delete_message(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = MESSAGE;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}

//logout
void send_delete_auth(struct dc_env *env, struct dc_error *err, int fd, const char * body) {
    DC_TRACE(env);

    struct binary_header_field header;
    header.version = DEFAULT_VERSION;
    header.type = DESTROY;
    header.object = AUTH;
    header.body_size = dc_strlen(env, body);
    serialize_header(env, err, &header, fd, body);
}
