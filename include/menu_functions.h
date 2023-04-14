//
// Created by Vasily Shorin on 2023-04-14.
//

#ifndef PROCESS_SERVER_MENU_FUNCTIONS_H
#define PROCESS_SERVER_MENU_FUNCTIONS_H

#include <dc_env/env.h>
#include <dc_error/error.h>
#include <ncurses.h>

struct arg {
    struct dc_env *env;
    struct dc_error *error;
    int socket_fd;
};


void create_new_chat(struct dc_env *env, struct dc_error *err, int socket_fd);
void show_active_users(struct dc_env *env, struct dc_error *err, int socket_fd);
void show_active_chats(struct dc_env *env, struct dc_error *err, int socket_fd);
void display_settings(struct dc_env *env, struct dc_error *err, int socket_fd );
void handle_menu_selection(struct dc_env *env, struct dc_error *err, int socket_fd, int choice);

#endif //PROCESS_SERVER_MENU_FUNCTIONS_H
