#ifndef CLIENT1_CONSTRUCT_PACKET_H
#define CLIENT1_CONSTRUCT_PACKET_H

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

struct base_packet {
    unsigned int version : 4;
    unsigned int type : 4;
    uint8_t object;
    uint16_t size;
    uint8_t *body;
};

void create_user_dispatch(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, const char *login_token, const char *display_name, const char *password);
void create_channel_dispatch(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, const char *channel_name, const char *user_display_name, bool is_public);
uint8_t *serialize_packet(const struct dc_env *env, struct dc_error *err, const struct base_packet *packet, size_t *packet_size);
void deserialize_header(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, uint8_t *received_packet);
int send_packet(const struct dc_env *env, struct dc_error *err, struct base_packet *packet, char* server_ip, int server_port);
ssize_t write_fully(int fd, const void *buffer, size_t len);
ssize_t read_fully(int fd, void *buffer, size_t len);

#endif //CLIENT1_CONSTRUCT_PACKET_H
