#include "handling-response.h"
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_string.h>
#include <dc_posix/dc_unistd.h>
#include <dlfcn.h>
#include <dc_util/system.h>
#include <dc_util/types.h>

#define BASE 10

void handle_server_ping_user(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

void handle_server_create(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_read(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_update(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);
void handle_server_delete(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body);

void handle_create_user_response(struct server_options *options, char *body);
void handle_create_channel_response(struct server_options *options, char *body);
void handle_create_message_response(struct server_options *options, char *body);
void handle_create_auth_response(struct server_options *options, char *body);

void handle_read_user_response(struct server_options *options, char *body);
void handle_read_channel_response(struct server_options *options, char *body);
void handle_read_message_response(struct server_options *options, char *body);
void handle_read_auth_response(struct server_options *options, char *body);


void handle_update_user_response(struct server_options *options, char *body);
void handle_update_channel_response(struct server_options *options, char *body);
void handle_update_message_response(struct server_options *options, char *body);
void handle_update_auth_response(struct server_options *options, char *body);

void handle_delete_user_response(struct server_options *options, char *body);
void handle_delete_channel_response(struct server_options *options, char *body);
void handle_delete_message_response(struct server_options *options, char *body);
void handle_delete_auth_response(struct server_options *options, char *body);

void handle_server_request(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->type)
    {
        case CREATE:
            handle_server_create(options, binaryHeaderField, body);
            break;
        case READ:
            handle_server_read(options, binaryHeaderField, body);
            break;
        case UPDATE:
            handle_server_update(options, binaryHeaderField, body);
            break;
        case DESTROY:
            handle_server_delete(options, binaryHeaderField, body);
            break;
        default:
            break;
    }
}


//handling the registering of a user
void handle_create_user_response(struct server_options *options, char *body)
{
    fprintf(options->log_file, "HANDLING CREATE USER RESPONSE\n");
    clear_debug_buffer(options->log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0) {
        fprintf(options->log_file, "Fields are invalid\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "409") == 0) {
        fprintf(options->log_file, "NON UNIQUE CREDENTIALS\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Username or email already exists\n", dc_strlen(options->env, "Username or email already exists\n"));
    } else if (dc_strcmp(options->env, response_code, "201") == 0) {
        fprintf(options->log_file, "CREATE USER SUCCESS\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK\n"));
    } else {
        fprintf(options->log_file, "RESPONSE CODE DOES NOT MATCH\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}

// handle login response from server
void handle_create_auth_response(struct server_options *options, char *body)
{
    fprintf(options->log_file, "HANDLING LOGIN RESPONSE\n");
    clear_debug_buffer(options->log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->log_file, "Fields are invalid\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->log_file, "User account not found\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "User account not found\n", dc_strlen(options->env, "User account not found\n"));
    } else if (dc_strcmp(options->env, response_code, "200") == 0)
    {
        char buffer[DEFAULT_SIZE];
        char * display_name = dc_strtok(options->env, NULL, "\3");
        char * privilege_level = dc_strtok(options->env, NULL, "\3");
        char * channel_name_list_size = dc_strtok(options->env, NULL, "\3");

        fprintf(options->log_file, "CREATE USER AUTH SUCCESS\n");
        fprintf(options->log_file, "DisplayName: %s\n", display_name);
        clear_debug_buffer(options->log_file);

        dc_strcpy(options->env, buffer, "OK ");

        dc_strcat(options->env, buffer, dc_strtok(options->env, "Test", "\3"));

        dc_strcat(options->env, buffer, "\0");
        fprintf(options->log_file, "UI RESPONSE: %s\n", buffer);
        clear_debug_buffer(options->log_file);

        write(STDOUT_FILENO, buffer, dc_strlen(options->env, buffer));
    } else {
        fprintf(options->log_file, "RESPONSE CODE DOES NOT MATCH\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}

//handling the channel creation response from the server
void handle_create_channel_response(struct server_options *options, char *body)
{
    fprintf(options->log_file, "HANDLING CREATE CHANNEL RESPONSE\n");
    clear_debug_buffer(options->log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->log_file, "Fields are invalid\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->log_file, "User account not found\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "User account not found\n", dc_strlen(options->env, "User account not found\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->log_file, "Name does not match\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Sender name does Not match Display Name\n", dc_strlen(options->env, "Sender name does Not match Display Name\n"));
    } else if (dc_strcmp(options->env, response_code, "409") == 0)
    {
        fprintf(options->log_file, "Channel Name Not UNIQUE\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Channel Name Not UNIQUE\n", dc_strlen(options->env, "Channel Name Not UNIQUE\n"));
    } else if (dc_strcmp(options->env, response_code, "201") == 0)
    {
        fprintf(options->log_file, "CREATE CHANNEL SUCCESS\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK\n"));
    } else {
        fprintf(options->log_file, "RESPONSE CODE DOES NOT MATCH\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }

}

//handling the channel join response from the server
void handle_create_message_response(struct server_options *options, char *body)
{
    // display-name ETX channel-name ETX message-content ETX timestamp ETX

    fprintf(options->log_file, "HANDLING CREATE MESSAGE RESPONSE\n");
    clear_debug_buffer(options->log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->log_file, "Fields are invalid\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->log_file, "CHANNEL NOT FOUND\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "CHANNEL NOT FOUND\n", dc_strlen(options->env, "CHANNEL NOT FOUND\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->log_file, "DISPLAY NAMES DONT MATCH\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "DISPLAY NAMES DONT MATCH\n", dc_strlen(options->env, "DISPLAY NAMES DONT MATCH\n"));
    } else if (dc_strcmp(options->env, response_code, "201") == 0)
    {
        fprintf(options->log_file, "CREATE MESSAGE SUCCESS\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "OK2\n", dc_strlen(options->env, "OK\n"));
    }  else
    {
        fprintf(options->log_file, "Server sent message\n");
        clear_debug_buffer(options->log_file);

        char buffer[DEFAULT_SIZE];
        dc_strtok(options->env, NULL, "\3");
        char * message_content = dc_strtok(options->env, NULL, "\3");
        char * time_stamp = dc_strtok(options->env, NULL, "\3");

        dc_strcpy(options->env, buffer, response_code);
        dc_strcat(options->env, buffer, "\3");
        dc_strcat(options->env, buffer, message_content);
        dc_strcat(options->env, buffer, "\3");
        dc_strcat(options->env, buffer, time_stamp);
        dc_strcat(options->env, buffer, "\3");
        dc_strcat(options->env, buffer, "\0");

        fprintf(options->log_file, "Message Received %s\n\n", buffer);
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, buffer, dc_strlen(options->env, buffer));
        clear_debug_buffer(options->log_file);
    }
}

void handle_server_create(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            handle_create_user_response(options, body);
            break;
        case CHANNEL:
            handle_create_channel_response(options, body);
            break;
        case MESSAGE:
            handle_create_message_response(options, body);
            break;
        case AUTH:
            handle_create_auth_response(options, body);
            break;
        default:
            break;
    }
}


//handling the message read response from the server
void handle_read_message_response(struct server_options *options, char *body) {

    fprintf(options->log_file, "HANDLING READ MESSAGE RESPONSE\n");
    clear_debug_buffer(options->log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0) {
        fprintf(options->log_file, "Fields are invalid\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0) {
        fprintf(options->log_file, "Channel NOT FOUND\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Channel NOT FOUND\n", dc_strlen(options->env, "Channel NOT FOUND\n"));
    } else if ((dc_strcmp(options->env, response_code, "200") == 0) || (dc_strcmp(options->env, response_code, "206") == 0)) {
        fprintf(options->log_file, "Received All Counts\n");
        clear_debug_buffer(options->log_file);

        char buffer[DEFAULT_SIZE];
        char * message_size = dc_strtok(options->env, NULL, "\3");

        fprintf(options->log_file, "CREATE USER AUTH SUCCESS\n");
        fprintf(options->log_file, "Message NUMBER: %s\n", message_size);
        clear_debug_buffer(options->log_file);

        uint16_t channel_size = dc_uint16_from_str(options->env, options->err, message_size, BASE);
        dc_strcpy(options->env, buffer, "OK ");

        for (int i = 0; i < channel_size; i++)
        {
            // message-info = display-name ETX channel-name ETX message-content ETX timestamp ETX
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
            dc_strcat(options->env, buffer, " ");
            dc_strcat(options->env, buffer, dc_strtok(options->env, NULL, "\3"));
        }

        dc_strcat(options->env, buffer, "\0");
        clear_debug_buffer(options->log_file);

        write(STDOUT_FILENO, buffer, dc_strlen(options->env, buffer));
    } else {
        fprintf(options->log_file, "RESPONSE CODE DOES NOT MATCH\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}

void handle_server_read(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            handle_read_user_response(options, body);
            break;
        case CHANNEL:
            handle_read_channel_response(options, body);
            break;
        case MESSAGE:
            handle_read_message_response(options, body);
            break;
        case AUTH:
            handle_read_auth_response(options, body);
            break;
        default:
            break;
    }
}
//A Client MUST send a Read–Message Request when it joins a Channel.
//If an implementation does not remove Users with an Online Status of “0” from a Channel’s List of Users,
//the implementation MUST provide a way for the Client to send a Read–Message Request when a User’s Online Status changes to “1”.

void handle_update_channel_response(struct server_options *options, char *body) {
    fprintf(options->log_file, "HANDLING UPDATE CHANNEL RESP\n");
    clear_debug_buffer(options->log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0) {
        fprintf(options->log_file, "Fields are invalid\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0) {
        fprintf(options->log_file, "Channel or User Does not Exist\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Channel Does not Exist\n", dc_strlen(options->env, "Channel Does not Exist\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0) {
        fprintf(options->log_file, "Sender Name Does NOT match Display Name\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Sender Name Does NOT match Display Name\n", dc_strlen(options->env, "Sender Name Does NOT match Display Name\n"));
    } else if (dc_strcmp(options->env, response_code, "200") == 0) {
        fprintf(options->log_file, "UPDATE CHANNEL SUCCESS\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK\n"));
    } else {
        fprintf(options->log_file, "INCORRECT RESPONSE CODE\n\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}


//server option
void handle_server_update(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            handle_update_user_response(options, body);
            break;
        case CHANNEL:
            handle_update_channel_response(options, body);
            break;
        case MESSAGE:
            handle_update_message_response(options, body);
            break;
        case AUTH:
            handle_update_auth_response(options, body);
            break;
        default:
            break;
    }
}

//logout
void handle_delete_auth_response(struct server_options *options, char *body) {

    fprintf(options->log_file, "HANDLING LOGOUT RESPONSE\n");
    clear_debug_buffer(options->log_file);

    char * response_code = dc_strtok(options->env, body, "\3");

    if (dc_strcmp(options->env, response_code, "400") == 0)
    {
        fprintf(options->log_file, "Fields are invalid\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Fields are invalid\n", dc_strlen(options->env, "Fields are invalid\n"));
    } else if (dc_strcmp(options->env, response_code, "404") == 0)
    {
        fprintf(options->log_file, "USER NOT FOUND\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "USER NOT FOUND\n", dc_strlen(options->env, "USER NOT FOUND\n"));
    } else if (dc_strcmp(options->env, response_code, "403") == 0)
    {
        fprintf(options->log_file, "DISPLAY NAMES DONT MATCH\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "DISPLAY NAMES DONT MATCH\n", dc_strlen(options->env, "DISPLAY NAMES DONT MATCH\n"));
    } else if (dc_strcmp(options->env, response_code, "412") == 0)
    {
        fprintf(options->log_file, "Already Successfully logged off\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "Already Exited\n", dc_strlen(options->env, "Already Exited\n"));
    } else if (dc_strcmp(options->env, response_code, "200") == 0)
    {
        fprintf(options->log_file, "Destroy Auth (Log out) SUCCESS\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "OK\n", dc_strlen(options->env, "OK\n"));
    }  else
    {
        fprintf(options->log_file, "RESPONSE CODE DOES NOT MATCH\n");
        clear_debug_buffer(options->log_file);
        write(STDOUT_FILENO, "SERVER ERROR\n", dc_strlen(options->env, "SERVER ERROR\n"));
    }
}


void handle_server_delete(struct server_options * options, struct binary_header_field * binaryHeaderField, char * body) {
    switch (binaryHeaderField->object)
    {
        case USER:
            handle_delete_user_response(options, body);
            break;
        case CHANNEL:
            handle_delete_channel_response(options, body);
            break;
        case MESSAGE:
            handle_delete_message_response(options, body);
            break;
        case AUTH:
            handle_delete_auth_response(options, body);
            break;
        default:
            break;
    }
}
