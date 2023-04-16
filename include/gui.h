//
// Created by Vasily Shorin on 2023-04-14.
//

#ifndef PROCESS_SERVER_GUI_H
#define PROCESS_SERVER_GUI_H

#include <dc_env/env.h>
#include <dc_error/error.h>
#include <ncurses.h>

/**
 * Draws login window, takes user input and sends it to server.
 *
 * @param env dc_env
 * @param err dc_error
 * @param socket_fd socket file descriptor
 */
void draw_login_win(struct dc_env *env, struct dc_error *err, int socket_fd);

/**
 * Draws menu options
 *
 * @param highlight
 */
void draw_menu(WINDOW *win, int highlight, int number_of_items);

/**
 * Draws register window, takes user input and sends it to server.
 *
 * @param env dc_env
 * @param err dc_error
 * @param socket_fd socket file descriptor
 */
void draw_register_window(struct dc_env *env, struct dc_error *err, int socket_fd);

/**
 * Highlights menu option in menu window and draws it.
 *
 * @param win the window to draw on
 * @param y y coordinate
 * @param x x coordinate
 * @param str string to draw
 */
void apply_highlight(WINDOW *win, int y, int x, const char *str);

/**
 * Removes highlight from menu option in menu window and draws it.
 *
 * @param win the window to draw on
 * @param y y coordinate
 * @param x x coordinate
 * @param str string to draw
 */
void remove_highlight(WINDOW *win, int y, int x, const char *str);

/**
 * Quits the program.
 */
void quit(void);

/**
 * Initializes windows.
 */
void init_windows(int number_of_items);

/**
 * Initializes menu.
 */
void init_menu(int number_of_items);

/**
 * Initializes chat.
 */
void init_chat(void);

/**
 * Initializes input.
 */
void init_input(void);

/**
 * Gets response code.
 */
long get_response_code(struct dc_env *env, struct dc_error *err, int socket_fd);

/**
 * Cleans up the main windows.
 */
void clean_up(void);

void* input_handler(void* arg);

void handle_command(struct dc_env *env, struct dc_error *err, char* input_buffer, int socket_fd);
#endif //PROCESS_SERVER_GUI_H
