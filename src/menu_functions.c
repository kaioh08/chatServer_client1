//
// Created by Vasily Shorin on 2023-04-14.
//

#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <menu.h>
#include "menu_functions.h"
#include "processor_utility.h"
#include "gui.h"

#define SERVER_PORT 5432
#define MAX_SIZE 1024
#define INPUT_HEIGHT 3
#define MENU_WIDTH 30
#define MAX_NAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20
#define MENU_ITEMS 5
#define MAX_SIZE 1024
char *display_name;
char *current_chat;
pthread_mutex_t mutex;

//WINDOW *menu_win, *chat_win, *input_win, *login_win, *register_win;


void create_new_chat(struct dc_env *env, struct dc_error *err, int socket_fd) {
    WINDOW *create_chat_win;
    bool is_private = false;
    char ETX[3] = "\x03";
    bool menu_focused = false;
    char chat_name[MAX_NAME_LENGTH + 1] = {0};
    char password[MAX_PASSWORD_LENGTH + 1] = {0};
    int create_chat_win_width = 50;
    int create_chat_win_height = 12;
    int startx = (COLS - create_chat_win_width) / 2;
    int starty = (LINES - create_chat_win_height) / 2;
    current_chat = dc_malloc(env, err, MAX_NAME_LENGTH + 1);
    create_chat_win = newwin(create_chat_win_height, create_chat_win_width, starty, startx);
    box(create_chat_win, 0, 0);
    wrefresh(create_chat_win);

// Labels
    mvwprintw(create_chat_win, 2, 2, "Chat Name:");
    mvwprintw(create_chat_win, 4, 2, "Public (y/n): ");
// Buttons
    mvwprintw(create_chat_win, 9, 10, "[Save]");
    mvwprintw(create_chat_win, 9, 28, "[Back]");
    bool quit = false;
    int ch;
    int current_field = 0;
    bool button_focused = false;

    keypad(create_chat_win, TRUE); // Enable keypad mode to capture arrow keys
    MEVENT event;
    while (!quit) {
        ch = wgetch(create_chat_win);
        // Get the cursor position
        int cur_y, cur_x;
        getyx(create_chat_win, cur_y, cur_x);
        if (!button_focused) {
            switch (ch) {
                // if escape is pressed, quit the loop
                case 27:
                    quit = true;
                    break;
                case KEY_DOWN:
                    if (current_field < 1) {
                        current_field++;
                    }
                    break;
                case KEY_UP:
                    if (current_field > 0) {
                        current_field--;
                    }
                    break;
                case KEY_LEFT:
                case KEY_RIGHT:
                    if (current_field == 1) {
                        is_private = !is_private;
                        mvwprintw(create_chat_win, 4, 15, "%c", is_private ? 'y' : 'n');
                    }
                    break;
                case KEY_BACKSPACE:
                case 127:
                    if (current_field == 0) {
                        if (dc_strlen(env, chat_name) > 0) {
                            chat_name[dc_strlen(env, chat_name) - 1] = '\0';
                        }
                    }
                    break;
                case '\n':
                case '\r':
                    if (cur_x >= 10 && cur_x <= 15) {
                        // Save button
                        // TODO: Add create server dispatch request
                        char publicity = is_private ? '0' : '1';
                        char request_body[256] = {0};
                        snprintf(request_body, 256, "%s%s%s%s%c%s", chat_name, ETX, display_name, ETX, publicity, ETX);
                        send_create_channel(env, err, socket_fd, request_body);
                        wprintw(create_chat_win, "Chat name: %s, publicity: %c", chat_name, publicity);
//                        if(
                        current_chat = strdup(chat_name);
                        wrefresh(create_chat_win);
                        refresh();
                        sleep(1);
                        // You can process the data and send it to the server here
                        quit = true;
                    } else if (cur_x >= 28 && cur_x <= 33) {
                        // Back button
                        quit = true;
                    }
                    break;
                default:
                    if (current_field == 0 && strlen(chat_name) < MAX_NAME_LENGTH) {
                        chat_name[strlen(chat_name)] = ch;
                    }
                    break;
            }
        }
        if ((cur_y == 9 && cur_x >= 10 && cur_x <= 15) || (cur_y == 9 && cur_x >= 28 && cur_x <= 33)) {
            if (!button_focused) {
                button_focused = true;
                apply_highlight(create_chat_win, 9, cur_x, "[Save]");
                apply_highlight(create_chat_win, 9, cur_x, "[Back]");
            }
        } else {
            if (button_focused) {
                button_focused = false;
                remove_highlight(create_chat_win, 9, 10, "[Save]");
                remove_highlight(create_chat_win, 9, 28, "[Back]");
            }
        }
        if (current_field == 0) {
            mvwprintw(create_chat_win, 2, 15, "%s", chat_name);
            wmove(create_chat_win, 2, 15 + strlen(chat_name));
        } else if (current_field == 1) {
            mvwprintw(create_chat_win, 4, 15, "%c", is_private ? 'y' : 'n');
            wmove(create_chat_win, 4, 15);
        }
        wrefresh(create_chat_win);
    }

    // Return focus to the menu window
    delwin(create_chat_win);
    clean_up();
    menu_focused = true;
    refresh();
}

void show_active_users(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    int startx, starty, width, height;
    height = 20;
    width = 50;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;
    WINDOW *active_users_win = newwin(20, 50, starty, startx);
    box(active_users_win, 0, 0);
    wrefresh(active_users_win);

    for(int i = 0; i < 10; i++)
    {
        mvwprintw(active_users_win, i+1, 1, "User %d", i);
    }

    int ch;
    bool quit = false;
    keypad(active_users_win, TRUE);

    while (!quit) {
        ch = wgetch(active_users_win);
        switch (ch) {
            case 27:
                quit = true;
                break;
        }
    }
    delwin(active_users_win);
    clean_up();
//    menu_focused = true;
}

void display_settings(struct dc_env *env, struct dc_error *err, int socket_fd) {
    // create a new window for the display name update form
    WINDOW *update_win = newwin(10, 60, (LINES - 10) / 2, (COLS - 60) / 2);
    box(update_win, 0, 0);

    // create input field for the new display name
    char display_name[MAX_NAME_LENGTH + 1] = {0};
    mvwprintw(update_win, 2, 2, "New Display Name: ");
    wmove(update_win, 2, 20);
    echo();
    curs_set(1);
    wgetnstr(update_win, display_name, MAX_NAME_LENGTH);
    noecho();
    curs_set(0);

    // create "Save" and "Cancel" buttons
    mvwprintw(update_win, 4, 20, "[ Save ]");
    mvwprintw(update_win, 4, 35, "[ Cancel ]");

    // allow user to click on the buttons
    keypad(update_win, TRUE);

    bool quit = false;
    int ch;
    bool save_clicked = false;
    while (!quit) {
        ch = wgetch(update_win);
        switch (ch) {
            case 27: // Escape
            case KEY_BACKSPACE:
                quit = true;
                break;
            case KEY_LEFT:
            case KEY_RIGHT:
                // toggle focus between buttons
                save_clicked = !save_clicked;
                break;
            case 10: // Enter
                if (save_clicked && strlen(display_name) > 0) {
                    // call create_update_user to update the user's display name
//                        create_update_user(env, err, fd, display_name);
//                    wprintw(chat_win, "Display name updated to %s", display_name);
                    sleep(2);
                    quit = true;
                } else {
                    quit = true;
                }
                break;
            default:
                break;
        }

        // highlight the active button
        if (save_clicked) {
            wattron(update_win, A_REVERSE);
            mvwprintw(update_win, 4, 20, "[ Save ]");
            wattroff(update_win, A_REVERSE);
            mvwprintw(update_win, 4, 35, "[ Cancel ]");
        } else {
            mvwprintw(update_win, 4, 20, "[ Save ]");
            wattron(update_win, A_REVERSE);
            mvwprintw(update_win, 4, 35, "[ Cancel ]");
            wattroff(update_win, A_REVERSE);
        }

        wrefresh(update_win);
    }

    delwin(update_win);
    clean_up();
//    menu_focused = true;
}

void show_active_chats(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    int startx, starty, width, height;
    height = 20;
    width = 50;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;
    WINDOW *list_of_users_win = newwin(20, 50, starty, startx);
    box(list_of_users_win, 0, 0);
    wrefresh(list_of_users_win);

    for(int i = 0; i < 10; i++)
    {
        mvwprintw(list_of_users_win, i+1, 1, "Chat %d", i);
    }

    int ch;
    bool quit = false;
    keypad(list_of_users_win, TRUE);

    while (!quit) {
        ch = wgetch(list_of_users_win);
        switch (ch) {
            case 27:
                quit = true;
                break;
        }
    }
    delwin(list_of_users_win);
    clean_up();
//    menu_focused = true;
}

void handle_menu_selection(struct dc_env *env, struct dc_error *err, int socket_fd, int choice) {
    switch (choice) {
        case 0: // Create new chat
            create_new_chat(env, err, socket_fd);
            break;
        case 1: // Show list of active chats
            show_active_chats(env, err, socket_fd);
            break;
        case 2: // Show active users
            show_active_users(env, err, socket_fd);
            break;
        case 3: // Settings
            display_settings(env, err, socket_fd);
            break;
        case 4:
            quit();
            break;
    }
}