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

/**
 * Draws create new chat window, takes user input channel name, publicity, and sends it to server.
 *
 * @param env dc_env
 * @param err dc_error
 * @param socket_fd socket file descriptor
 */
void create_new_chat(struct dc_env *env, struct dc_error *err, int socket_fd);

/**
 * Draws active user list windows.
 *
 * @param env dc_env
 * @param err dc_error
 * @param socket_fd socket file descriptor
 */
void show_active_users(struct dc_env *env, struct dc_error *err, int socket_fd);

/**
 * Draws active chat list windows.
 *
 * @param env dc_env
 * @param err dc_error
 * @param socket_fd socket file descriptor
 */
void show_active_chats(struct dc_env *env, struct dc_error *err, int socket_fd);

/**
 * Draws user settings window.
 *
 * @param env dc_env
 * @param err dc_error
 * @param socket_fd socket file descriptor
 */
void display_settings(struct dc_env *env, struct dc_error *err, int socket_fd );

/**
 * Handles menu selection, calls appropriate function.
 *
 * @param env dc_env
 * @param err dc_err
 * @param socket_fd socket file descriptor
 * @param choice menu option
 */
void handle_menu_selection(struct dc_env *env, struct dc_error *err, int socket_fd, int choice);

#endif //PROCESS_SERVER_MENU_FUNCTIONS_H
