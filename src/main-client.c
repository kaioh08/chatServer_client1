#include "../include/processor_utility.h"
#include "global_vars.h"

// create a struct called login_info and put the username and channel size, channel names in the struct at the
// moment of login, THEN  if the list of is changed, update the struct
struct login_info {
    char * username;
    unsigned int channel_size;
    char ** channel_names;
};

// to save the username and channel_size and channel_name after you login.
static void save_username(struct dc_env *env, struct dc_error *err, struct login_info * login, const char * username) {
    login->username = dc_strcpy(env, (char *) err, username);
    //const struct dc_env *env, char *restrict s1, const char *restrict s2, size_t n
}

// add more channels after last channel names (assuming the channel name cannot be duplicated)
static void add_channel(struct dc_env *env, struct dc_error *err, struct login_info * login, const char * channel_name) {
    login->channel_names[login->channel_size] = dc_strcpy(env, (char *) err, channel_name);
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


void deserialize_header(struct dc_env *env, struct dc_error *err, int fd, struct binary_header_field *b_header, uint32_t value) {
    uint32_t header2;

    // Convert to network byte order
    value = ntohl(value);

    b_header->version = (value >> 28) & 0x0F; // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    b_header->type = (value >> 24) & 0x0F;    // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    // Check if the type is any of the pings or not
    if (b_header->type != PINGUSER && b_header->type != PINGCHANNEL) {
        // read the remaining 3 bytes
        dc_read_fully(env, err, fd, &header2, 3);

        // Convert to network byte order
        header2 = ntohl(header2);

        b_header->object = (header2 >> 24) & 0xFF;  // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
        b_header->body_size = (header2 >> 8) & 0xFFFF;     // NOLINT(hicpp-signed-bitwise,cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    }

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

    pthread_mutex_lock(&socket_mutex);
    dc_write(env, err, fd, &data, (sizeof(uint32_t) + dc_strlen(env, body)));
    pthread_mutex_unlock(&socket_mutex);
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
