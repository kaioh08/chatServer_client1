#include "handling-response.h"
#include "global_vars.h"
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_string.h>
#include <dc_posix/dc_unistd.h>
#include <dlfcn.h>
#include <dc_util/system.h>
#include <dc_util/types.h>
#include <pthread.h>

#define BASE 10

void write_simple_debug_msg(FILE *file, const char *str)
{
    pthread_mutex_lock(&debug_file_mutex);
    fprintf(file, "%s\n", str);
    clear_debug_file_buffer(file);
    pthread_mutex_unlock(&debug_file_mutex);
}

void clear_debug_file_buffer(FILE * debug_log_file)
{
    fflush(debug_log_file);
    setbuf(debug_log_file, NULL);
}

//void *read_message_handler(void *arg)
//{
//    struct dc_env *env;
//    struct dc_error *err;
//    int fd;
//    char *response_buffer;
//    struct binary_header_field *b_header;
//    FILE *file;
//    struct read_handler_args *args = (struct read_handler_args *) arg;
//    env = args->env;
//    err = args->err;
//    fd = args->socket_fd;
//    response_buffer = args->response_buffer;
//    b_header = args->b_header;
//    file = args->debug_log_file;
//    DC_TRACE(env);
//
//    write_simple_debug_msg(file, "read_message_handler started\n");
//    fd_set read_fds;
//    struct timeval tv;
//    int ret;
//
//    tv.tv_sec = 0;
//    tv.tv_usec = 5000;
//
//    ssize_t nread;
//
//    while(true)
//    {
//        FD_ZERO(&read_fds);
//        FD_SET(fd, &read_fds);
////        write_simple_debug_msg(file, "Anything new?\n");
//
//        ret = select(fd + 1, &read_fds, NULL, NULL, &tv);
//
//        if (ret == -1) {
//            write_simple_debug_msg(file, "Select failed\n");
//        }
//        else if (ret == 0)
//        {
//            // Timeout expired
////            write_simple_debug_msg(file, "Timeout\n");
//        }
//        else
//        {
//            uint32_t unprocessed_binary_header;
//            while(response_buffer_updated == 1);
//            pthread_mutex_lock(&socket_mutex);
//            nread = dc_read(env, err, fd, &unprocessed_binary_header, sizeof(uint8_t)); //depending on how deserialize_header() works, the nbytes might have to change
//            if (nread < 0) {
//                write_simple_debug_msg(file, "Read failed\n");
//            }
//            else if (nread == sizeof(uint8_t))
//            {
//                write_simple_debug_msg(file, "got sth\n");
//                //when fd has stuff, read the first few bytes to get the header fields
//                pthread_mutex_lock(&response_buffer_mutex);
//                deserialize_header(env, err, fd, b_header, unprocessed_binary_header);
//                //read the dispatch after getting the binary header
//                char buffer[1024];
//                nread = dc_read_fully(env, err, fd, buffer, b_header->body_size);
//                buffer[b_header->body_size] = '\0';
//                strcpy(response_buffer, buffer);
//                pthread_mutex_unlock(&socket_mutex);
//                pthread_mutex_lock(&debug_file_mutex);
//                fprintf(file, "Received response:\nversion: %d\ntype: %d\nobject: %hhu\nbody size: %d\nbody: %s\n",
//                        b_header->version, b_header->type, b_header->object, b_header->body_size, response_buffer);
//                clear_debug_file_buffer(file);
//                pthread_mutex_unlock(&debug_file_mutex);
//                response_buffer_updated = 1;
//                pthread_mutex_unlock(&response_buffer_mutex);
//            }
//        }
//
//    }
//}

void response_handler_wrapper(struct dc_env *env, struct dc_error *err, struct arg *options, struct binary_header_field *b_header, char *body)
{
    switch(b_header->type)
    {
        case CREATE:
        {
            write_simple_debug_msg(options->debug_log_file, "\ncalling handle_server_create()\n");
            handle_server_create(options, b_header, body);
            break;
        }
        case READ:
        {
            write_simple_debug_msg(options->debug_log_file, "\ncalling handle_server_read()\n");
            handle_server_read(options, b_header, body);
            break;
        }
        case UPDATE:
        {
//          TODO:  handle_server_update(options, b_header, body);
            write_simple_debug_msg(options->debug_log_file, "\ncannot call handle_server_update()! (not implemented)\n");
            handle_server_update(options, b_header, body);
            break;
        }
        case DESTROY:
        {
//          TODO:  handle_server_delete(options, b_header, body);
            write_simple_debug_msg(options->debug_log_file, "\ncannot call handle_server_delete()! (not implemented)\n");
            break;
        }
        default:
        {
            write_simple_debug_msg(options->debug_log_file, "\nbad type\n");
        }
    }
}


void handle_server_request(struct arg * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->type)
    {
        case CREATE:
            handle_server_create(options, binaryHeaderField, body);
            break;
        case READ:
            handle_server_read(options, binaryHeaderField, body);
            break;
        case UPDATE:
//          TODO:  handle_server_update(options, binaryHeaderField, body);
            break;
        case DESTROY:
//          TODO:  handle_server_delete(options, binaryHeaderField, body);
            break;
        case PINGUSER:
//          TODO:  handle_server_ping_user(options, binaryHeaderField, body);
            break;
        default:
            break;
    }
}


/**
 * Handle CREATE STUFF
 */

int handle_create_user_response(struct arg *options, char *body)
{
    // 400 409 201

    write_simple_debug_msg(options->debug_log_file, "HANDLING CREATE USER RESP\n");

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0) {
        write_simple_debug_msg(options->debug_log_file, "Fields are invalid\n");
        return BAD_REQUEST;
    } else if (dc_strcmp(options->env, response_code, "409") == 0) {
        write_simple_debug_msg(options->debug_log_file, "NON UNIQUE CREDENTIALS\n");
        return NOT_UNIQUE;
    } else if (dc_strcmp(options->env, response_code, "201") == 0) {
        write_simple_debug_msg(options->debug_log_file, "CREATE USER SUCCESS\n");
        return CREATED;
    } else {
        write_simple_debug_msg(options->debug_log_file, "INCORRECT RESPONSE CODE\n");
        return -1;
    }
}

int handle_create_auth_response(struct arg *options, char *body)
{
    // 400 403 200
    write_simple_debug_msg(options->debug_log_file, "HANDLING CREATE USER AUTH RESP\n");

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "Fields are invalid\n");
        return BAD_REQUEST;
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "User account not found\n");
        return FORBIDDEN;
    } else if (dc_strcmp(options->env, response_code, "200") == 0)
    {
        char buffer[1024];
        // “200” ETX display-name ETX privilege-level ETX channel-name-list
        char * d_name = dc_strtok(options->env, NULL, "\3");
        if(d_name != NULL)
        {
            strcpy(display_name, d_name);
        }
        char *privilege_level = dc_strtok(options->env, NULL, "\3");
        char *channel_name_list_size_str = dc_strtok(options->env, NULL, "\3");

        fprintf(options->debug_log_file, "CREATE USER AUTH SUCCESS\n");
        fprintf(options->debug_log_file, "DisplayName: %s\n", display_name);
        fprintf(options->debug_log_file, "PrivilegeLevel: %s\n", privilege_level);
        fprintf(options->debug_log_file, "ChannelNameListSize: %s\n", channel_name_list_size_str);
        clear_debug_file_buffer(options->debug_log_file);

        // OK GLOBAL, Channel1\0
        dc_strcpy(options->env, buffer, "OK ");

        int channel_name_list_size = atoi(channel_name_list_size_str);
        for (int i = 0; i < channel_name_list_size; i++)
        {
            char *channel_name = dc_strtok(options->env, NULL, "\3");
            if (channel_name != NULL)
            {
                if (i == 0) // first channel name in the list is the current chat
                {
                    strcpy(current_channel, channel_name);
                }
                dc_strcat(options->env, buffer, channel_name);
                dc_strcat(options->env, buffer, ", ");
            }
        }

        dc_strcat(options->env, buffer, "\0");
        fprintf(options->debug_log_file, "UI RESPONSE: %s\n", buffer);
        clear_debug_file_buffer(options->debug_log_file);

        write(STDOUT_FILENO, buffer, dc_strlen(options->env, buffer));
        return ALL_GOOD;
    }
    else
    {
        write_simple_debug_msg(options->debug_log_file, "INCORRECT RESPONSE CODE\n");
        return -1;
    }


}

int handle_create_channel_response(struct arg *options, char *body)
{
    // 400 404 403 409 201
    write_simple_debug_msg(options->debug_log_file, "HANDLING CREATE CHANNEL RESP\n");

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "Fields are invalid\n");
        return BAD_REQUEST;
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "User account not found\n");
        return NOT_FOUND;
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "Sender name does Not match Display Name\n");
        return FORBIDDEN;
    } else if (dc_strcmp(options->env, response_code, "409") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "Channel Name Not UNIQUE\n");
        return NOT_UNIQUE;
    } else if (dc_strcmp(options->env, response_code, "201") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "CREATE CHANNEL SUCCESS\n");
        return CREATED;
    } else {
        write_simple_debug_msg(options->debug_log_file, "INCORRECT RESPONSE CODE\n");
        return -1;
    }

}

int handle_create_message_response(struct arg *options, char *body)
{
    // 400 404 403 201

    write_simple_debug_msg(options->debug_log_file, "HANDLING CREATE MESSAGE RESP\n");

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "Fields are invalid\n");
        return BAD_REQUEST;
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "CHANNEL NOT FOUND\n");
        return NOT_FOUND;
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "DISPLAY NAMES DONT MATCH\n");
        return FORBIDDEN;
    } else if (dc_strcmp(options->env, response_code, "201") == 0)
    {
        write_simple_debug_msg(options->debug_log_file, "CREATE MESSAGE SUCCESS\n");
        return CREATED;
    }
    else
    {
        char buffer[1024];
        // It's  a message to be displayed on the UI
        // display-name ETX channel-name ETX message-content ETX timestamp ETX
        char * display_name = dc_strtok(options->env, body, "\3");
        dc_strtok(options->env, body, "\3");
        char * message_content = dc_strtok(options->env, body, "\3");
        dc_strtok(options->env, body, "\3");

        // Create the body
        dc_strcpy(options->env, buffer, display_name);
        dc_strcat(options->env, buffer, " ");
        dc_strcat(options->env, buffer, message_content);
        dc_strcat(options->env, buffer, "\0");

        fprintf(options->debug_log_file, "Message Received %s\n", buffer);
        clear_debug_file_buffer(options->debug_log_file);
    }
}

void handle_server_create(struct arg * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            write_simple_debug_msg(options->debug_log_file, "\ncalling handle_create_user_response\n");
            handle_create_user_response(options, body);
            break;
        case CHANNEL:
            write_simple_debug_msg(options->debug_log_file, "\ncalling handle_create_channel_response\n");
            handle_create_channel_response(options, body);
            break;
        case MESSAGE:
            write_simple_debug_msg(options->debug_log_file, "\ncalling handle_create_message_response\n");
            handle_create_message_response(options, body);
            break;
        case AUTH:
            write_simple_debug_msg(options->debug_log_file, "\ncalling handle_create_auth_response\n");
            handle_create_auth_response(options, body);
            break;
        default:
            break;
    }
}

/**
 * READ STUFF
 */


void handle_read_message_response(struct arg *options, char *body) {
    fprintf(options->debug_log_file, "HANDLING READ MESSAGE RESPONSE\n");
    clear_debug_file_buffer(options->debug_log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0) {
        fprintf(options->debug_log_file, "INVALID FIELD\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "INVALID FIELD\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0) {
        fprintf(options->debug_log_file, "CHANNEL DOES NOT EXIST\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "CHANNEL DOES NOT EXIST\n", dc_strlen(options->env, "Channel NOT FOUND\n"));
    } else if ((dc_strcmp(options->env, response_code, "200") == 0) || (dc_strcmp(options->env, response_code, "206") == 0)) {
        fprintf(options->debug_log_file, "SUCCESS\n");
        clear_debug_file_buffer(options->debug_log_file);

        char buffer[1024];
        // read-message-partial-res-body = “206” ETX message-list
        char * message_size = dc_strtok(options->env, NULL, "\3");

        fprintf(options->debug_log_file, "CREATE_USER_AUTH SUCCESS\n");
        fprintf(options->debug_log_file, "Message NUMBER: %s\n", message_size);
        clear_debug_file_buffer(options->debug_log_file);

        uint16_t channel_size = dc_uint16_from_str(options->env, options->error, message_size, BASE);
        dc_strcpy(options->env, buffer, "OK ");

        for (int i = 0; i < channel_size; i++)
        {
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
        }

        dc_strcat(options->env, buffer, "\0");
        clear_debug_file_buffer(options->debug_log_file);

        write(STDOUT_FILENO, buffer, dc_strlen(options->env, buffer));
    } else {
        fprintf(options->debug_log_file, "ERROR IN RESPONSE CODE\n");
        clear_debug_file_buffer(options->debug_log_file);
        write(STDOUT_FILENO, "SERVER_SIDE ERROR\n", dc_strlen(options->env, "SERVER_SIDE ERROR\n"));
    }
}


void handle_server_read(struct arg * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
//            TODO: handle_read_user_response(options, body);
            break;
        case CHANNEL:
//            TODO: handle_read_channel_response(options, body);
            break;
        case MESSAGE:
            handle_read_message_response(options, body);
            break;
        case AUTH:
//            TODO: handle_read_auth_response(options, body);
            break;
        default:
            break;
    }
}

void handle_server_update(struct arg * options, struct binary_header_field * binaryHeaderField, char * body)
{
    switch (binaryHeaderField->object) {
        case USER:
            //TODO: handle_update_user_response(options, body);
            write_simple_debug_msg(options->debug_log_file, "handle_update_user_response not implemented!\n");
            break;
        case CHANNEL:
            handle_update_channel_response(options, body);
            break;
        case MESSAGE:
            //TODO: handle_update_message_response(options, body);
            write_simple_debug_msg(options->debug_log_file, "handle_update_message_response not implemented!\n");
            break;
        case AUTH:
            //TODO: handle_update_auth_response(options, body);
            write_simple_debug_msg(options->debug_log_file, "handle_update_auth_response not implmented!\n");
            break;
        default:
            break;
    }
}

int handle_update_channel_response(struct arg *options, char *body)
{
    //return response code; in input handler, display message associated with response code
}