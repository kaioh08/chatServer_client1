//
// Created by Vasily Shorin on 2023-03-07.
//

#ifndef NCRUSES_MAIN_H
#define NCRUSES_MAIN_H

#include <ncurses.h>
#include <time.h>

/**
 * Initializes the ncurses library and the main window.
 * Draws the main window.
 */
void init_ncurses();

/**
 * Cleans up the ncurses library and destroys the main window.
 */
void cleanup_ncurses();

/**
 * Initializes colors for the ncurses library.
 */
void init_colors();

/**
 * Draws the main window.
 * The main window is the entire screen.
 */
void draw_main_window();


/**
 * Prints the given message with a timestamp to the specific chat window.
 * Shows message in the following format: [HH:MM:SS] Sender: message
 *
 * @param chat_window The window to print the message to.
 * @param time_t The time to print.
 * @param sender The sender of the message.
 * @param message_length The length of the message.
 * @param message The message to print.
 */
void print_message(WINDOW *chat_window, time_t time, char *sender, size_t message_length, char *message);

/**
 * Prints the error message to the specific chat window.
 * Shows message in the following format: [HH:MM:SS] Error: message
 *
 * @param chat_window The window to print the message to.
 * @param time_t The time to print.
 * @param message_length The length of the message.
 * @param message The error message to print.
 */
void print_error(WINDOW *chat_window, time_t time, size_t message_length, char *message);

/**
 * Draws the input window at the bottom of the screen.
 * Created as a subwindow of the main window.
 * The input window is 3 lines high and the width of the main window.
 *
 * @param main_window The main window to draw the input window to.
 */
void draw_input_window();

/**
 * Draws the chat window at the top of the screen.
 * Created as a subwindow of the main window.
 * The chat window is 6 lines high and the width of the main window.
 *
 * @param main_window The main window to draw the chat window to.
 */
void draw_chat_window();

/**
 * Draws login window.
 * The login window is the entire screen. It is drawn in the middle of the screen.
 * Contains a login form: username and password. The form is 4 lines high and 20 characters wide.
 */
void draw_login_window();

/**
 * Draws the register window.
 * Takes the username, password and display name from the user.
 * Upon pressing the register button, the packet is sent to the server for registration.
 */
void draw_register_window();


/**
 * Draws menu window.
 * Menu is on the right side of the screen. It contains the list of chats and the list of users.
 * The menu window is 6 lines high and 20 characters wide.
 * The menu window is a subwindow of the main window.
 */
void draw_menu_window();


/**
 * Format the given time to the following format: [HH:MM:SS]
 */
void formatTime(time_t time, char *time_str);

/**
 * Draws the chat list window. Sends a request to the server for the list of chats.
 * The chat list window is a subwindow of the menu window.
 */
void draw_public_chat_window();

/**
 * Draws the user list window. Sends a request to the server for the list of users online.
 * The user list window is a subwindow of the menu window.
 * Is drawn on top menu window. Has a back button to go back to the menu window.
 */
void draw_user_list_window();

/**
 * Creates chat window. Sends a request to the server to create a chat.
 * The chat window is a subwindow of the menu window.
 * Is drawn on top menu window. Has a back button to go back to the menu window.
 */
void draw_create_chat_window();

/**
 * Draws user setting window. This window is used to change the user's settings.
 * This includes changing the user's name, password. Has save button under each input field.
 * Has a back button at the bottom of the window.
 */
void draw_user_settings_window();

#endif //NCRUSES_MAIN_H
