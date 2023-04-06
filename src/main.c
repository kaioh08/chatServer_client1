#include "../include/processor_utility.h"
#include <arpa/inet.h>
#include <dc_c/dc_stdio.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <dc_env/env.h>
#include <dc_error/error.h>
#include <dc_posix/dc_unistd.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

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

bool menu_focused = false;
int menu_highlight = 0;
bool menu_active = true;

struct arg {
    struct dc_env *env;
    struct dc_error *error;
    int socket_fd;
};

pthread_mutex_t mutex;
WINDOW *menu_win, *chat_win, *input_win, *login_win, *register_win;

void init_windows(void);
void generate_timestamp(char* timestamp, size_t len);
void init_menu(void);
void init_chat(void);
void init_input(void);
void* input_handler(void *arg);
void* message_handler(void* arg);
void draw_menu(int highlight);
void draw_register_window(struct dc_env *env, struct dc_error *err, int socket_fd);
void apply_highlight(WINDOW *win, int y, int x, const char *str);
void remove_highlight(WINDOW *win, int y, int x, const char *str);
void show_active_users(struct dc_env *env, struct dc_error *err, int socket_fd);
void show_active_chats(struct dc_env *env, struct dc_error *err, int socket_fd);
void draw_login_win(struct dc_env *env, struct dc_error *err, int socket_fd);
void handle_menu_selection(struct dc_env *env, struct dc_error *err, int socket_fd, int choice);
void display_settings(struct dc_env *env, struct dc_error *err, int socket_fd );
void quit();

long get_response_code(struct dc_env *env, struct dc_error *err, int socket_fd);

void draw_menu(int highlight) {
    const char *choices[] = {
            "Create new chat",
            "Show list of active chats",
            "Show active users",
            "Settings",
            "Quit",
    };

    for (int i = 0; i < MENU_ITEMS; i++) {
        if (highlight == i) {
            wattron(menu_win, A_REVERSE);
        }
        mvwprintw(menu_win, i + 1, 1, "%s", choices[i]);
        wattroff(menu_win, A_REVERSE);
    }
    wrefresh(menu_win);
}

void apply_highlight(WINDOW *win, int y, int x, const char *str) {
    wattron(win, A_REVERSE);
    mvwprintw(win, y, x, str);
    wattroff(win, A_REVERSE);
}

void remove_highlight(WINDOW *win, int y, int x, const char *str) {
    mvwprintw(win, y, x, str);
}

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
    current_chat = malloc(MAX_NAME_LENGTH + 1);
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
                        if (strlen(chat_name) > 0) {
                            chat_name[strlen(chat_name) - 1] = '\0';
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
    touchwin(menu_win);
    wrefresh(menu_win);
    touchwin(chat_win);
    wrefresh(chat_win);
    touchwin(input_win);
    wrefresh(input_win);
    menu_focused = true;
    refresh();
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
    touchwin(menu_win);
    wrefresh(menu_win);
    touchwin(chat_win);
    wrefresh(chat_win);
    touchwin(input_win);
    wrefresh(input_win);
    menu_focused = true;
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
    touchwin(menu_win);
    wrefresh(menu_win);
    touchwin(chat_win);
    wrefresh(chat_win);
    touchwin(input_win);
    wrefresh(input_win);
    menu_focused = true;
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
                        wprintw(chat_win, "Display name updated to %s", display_name);
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
        touchwin(menu_win);
        wrefresh(menu_win);
        touchwin(chat_win);
        wrefresh(chat_win);
        touchwin(input_win);
        wrefresh(input_win);
        menu_focused = true;
}


void* message_handler(void* arg) {
    while (true) {
        // TODO: Implement message sending and receiving
        usleep(100000); // Sleep to prevent high CPU usage
    }

    return NULL;
}

void draw_login_win(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    int x, y, login_x, login_y, password_x, password_y, login_len, password_len;
    char username_login[20], password[20];
    char ETX[3] = "\x03";
    char *buffer = malloc(sizeof(char) * MAX_SIZE);
    memset(buffer, 0, sizeof(char) * MAX_SIZE);
    login_win = newwin(10, 40, (LINES - 10) / 2, (COLS - 40) / 2);
    clear();

    getmaxyx(stdscr, y, x);

    login_len = strlen("Login: ");
    login_x = x / 2 - login_len;
    login_y = y / 2;
    password_len = strlen("Password: ");
    password_x = x / 2 - password_len;
    password_y = login_y + 1;

    mvprintw(login_y - 2, x / 2 - 10, "Please enter your credentials:");
    mvprintw(login_y, login_x, "Login: ");
    mvprintw(password_y, password_x, "Password: ");

    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
    mvprintw(password_y + 2, x / 2, "  Register  ");

    echo();
    move(login_y, login_x + login_len);
    refresh();
    getstr(username_login);


    move(password_y, password_x + password_len);
    refresh();
    echo();
    getstr(password);



    if(strlen(username_login) > 20)
    {
        mvprintw(password_y + 6, x / 2 - 15, "Error: username_login too long");
        quit();
    }
    if(strlen(password) < 6)
    {
        sleep(3);
        mvprintw(password_y + 6, x / 2 - 15, "Error: password too short");
//        quit();
    }

    strcat(buffer, username_login);
    strcat(buffer, ETX);
    strcat(buffer, password);
    strcat(buffer, ETX);
    buffer[strlen(buffer)] = '\0';



    // highlight login button
    attron(A_REVERSE);
    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
    attroff(A_REVERSE);

    //TODO: let user choose between login and register before entering credentials

    int focus = 0;
    bool done = false;
    int response;
    while (!done)
    {
        refresh();
        int ch = getch();

        switch (ch)
        {
            case KEY_DOWN:
                if (focus == 0)
                {
                    focus = 1;
                    // highlight register button
                    attron(A_REVERSE);
                    mvprintw(password_y + 2, x / 2, "  Register  ");
                    attroff(A_REVERSE);
                    // unhighlight login button
                    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
                    int newCh = getch();
                    if(newCh =='\n')
                    {
                        draw_register_window(env, err, socket_fd);
                        done = true;
                    }
                }
                break;
            case KEY_UP:
                if (focus == 1)
                {
                    focus = 0;
                    // highlight login button
                    attron(A_REVERSE);
                    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
                    attroff(A_REVERSE);
                    // unhighlight register button
                    mvprintw(password_y + 2, x / 2, "  Register  ");
                    int newCh = getch();
                    // if login button is pressed, call login function
                    if(newCh =='\n')
                    {
//                        send_create_auth(env, err, socket_fd, buffer);
                        done = true;
                    }
                }
                break;
            case '\n':
                if (strlen(buffer) < MAX_SIZE)
                {
                    send_create_auth(env, err, socket_fd, buffer);
                    long response = get_response_code(env, err, socket_fd);
                    if (response != 200)
                    {
                        mvprintw(y - 2, x / 2 - 15, "Error: invalid credentials");
                        wrefresh(login_win);
                        sleep(3);
                        draw_login_win(env, err, socket_fd);
                        done = true;
                    } else {
                        mvprintw(y - 2, x / 2 - 15, "Success: logged in");
                        wrefresh(login_win);
//                        display_name = malloc(sizeof(char) * strlen(username_login) + 1);
//                        strcpy(display_name, username_login);
                        init_windows();
                        refresh();
                        done = true;
                    }
//                    draw_register_window(env, err, socket_fd);
//                    done = true;
                } else
                {
                    // handle buffer overflow
                    mvprintw(y - 2, x / 2 - 15, "Error: buffer overflow");
                }
                break;
            default:
                // handle other key presses
                break;
        }
        // consume newline character
        if (ch != KEY_DOWN && ch != KEY_UP)
        {
            getch();
        }
    }

    free(buffer);
    noecho();
    refresh();
    delwin(login_win);
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

void init_windows(void) {
    init_menu();
    init_chat();
    init_input();
    refresh();
}

void init_menu(void) {
    menu_win = newwin(LINES - INPUT_HEIGHT, MENU_WIDTH, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(1));
    box(menu_win, 0, 0);
    wrefresh(menu_win);
    draw_menu(0);
    refresh();
}

void init_chat(void) {
    chat_win = newwin(LINES - INPUT_HEIGHT, COLS - MENU_WIDTH, 0, MENU_WIDTH);
    scrollok(chat_win, TRUE);
    box(chat_win, 0, 0);
    wrefresh(chat_win);
    refresh();
}

void init_input(void) {
    input_win = newwin(INPUT_HEIGHT, COLS, LINES - INPUT_HEIGHT, 0);
    keypad(input_win, TRUE);
    scrollok(input_win, TRUE);
    box(input_win, 0, 0);
    wrefresh(input_win);
    mvwprintw(input_win, 1, 1, "Enter message: ");
    wrefresh(input_win);
    nodelay(input_win, TRUE);
    refresh();
}

void draw_register_window(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    int x, y, username_x, username_y, password_x, password_y, displayname_x, displayname_y, username_len, password_len, displayname_len;
    char username_register[20], displayname[20], password[20];
    char ETX[3] = "\x03";
    char *buffer = malloc(sizeof(char) * MAX_SIZE);
    memset(buffer, 0, sizeof(char) * MAX_SIZE);
    register_win = newwin(12, 40, (LINES - 12) / 2, (COLS - 40) / 2);
    clear();

    getmaxyx(stdscr, y, x);

    username_len = strlen("Username: ");
    username_x = x / 2 - username_len;
    username_y = y / 2 - 1;
    password_len = strlen("Password: ");
    password_x = x / 2 - password_len;
    password_y = username_y + 2;
    displayname_len = strlen("Display Name: ");
    displayname_x = x / 2 - displayname_len;
    displayname_y = password_y + 2;

    mvprintw(username_y - 2, x / 2 - 10, "Please enter your registration details:");
    mvprintw(username_y, username_x, "Username: ");


    mvprintw(displayname_y, displayname_x, "Display Name: ");

    mvprintw(password_y, password_x, "Password: ");




    mvprintw(displayname_y + 2, x / 2 - 10, "  Register  ");
    mvprintw(displayname_y + 2, x / 2, "  Cancel  ");

    echo();
    move(username_y, username_x + username_len);
    refresh();
    getstr(username_register);

    move(password_y, password_x + password_len);
    refresh();
    echo();
    getstr(password);

    move(displayname_y, displayname_x + displayname_len);
    refresh();
    echo();
    getstr(displayname);

    if(strlen(username_register) > 20)
    {
        mvprintw(displayname_y + 6, x / 2 - 15, "Error: display_name too long");
        quit();
    }
    if(strlen(password) < 6)
    {
        mvprintw(displayname_y + 6, x / 2 - 15, "Error: password too short");
//        quit();
    }

    strcat(buffer, username_register);
    strcat(buffer, ETX);
    strcat(buffer, displayname);
    strcat(buffer, ETX);
    strcat(buffer, password);
    strcat(buffer, ETX);

    buffer[strlen(buffer)] = '\0';


    // highlight register button
    attron(A_REVERSE);
    mvprintw(displayname_y + 2, x / 2 - 10, "  Register  ");
    attroff(A_REVERSE);

    int focus = 0;
    bool done = false;


    while (!done)
    {
        refresh();
        int ch = getch();

        switch (ch)
        {
            case KEY_DOWN:
                if (focus == 0)
                {
                    focus = 1;
                    // highlight cancel button
                    attron(A_REVERSE);
                    mvprintw(displayname_y + 2, x / 2, "  Cancel  ");
                    attroff(A_REVERSE);
                    // unhighlight register button
                    mvprintw(displayname_y + 2, x / 2 - 10, "  Register  ");
                }
                break;
            case KEY_UP:
                if (focus == 1)
                {
                    focus = 0;
                    // highlight register
                    attron(A_REVERSE);
                    mvprintw(displayname_y + 2, x / 2 - 10, "  Register  ");
                    attroff(A_REVERSE);
                    // unhighlight cancel button
                    mvprintw(displayname_y + 2, x / 2, "  Cancel  ");
                    int newCh;
                    if(newCh == '\n')
                    {
                        send_create_user(env, err, socket_fd, buffer);
                        done = true;
                    }
                }
                break;
            case '\n':
                if (strlen(buffer) < MAX_SIZE)
                {
//                    init_windows();
                    send_create_user(env, err, socket_fd, buffer);
                    long response = get_response_code(env, err, socket_fd);
                    if(response != 201)
                    {
                        mvprintw(displayname_y + 6, x / 2 - 15, "Error: display_name already exists");
                        wrefresh(register_win);
                        sleep(2);
                        draw_register_window(env, err, socket_fd);
                        refresh();
                    }
                    else
                    {
                        mvprintw(displayname_y + 6, x / 2 - 15, "Registration successful");
                        wrefresh(register_win);
                        sleep(1);
                        draw_login_win(env, err, socket_fd);
                        refresh();
                        done=true;
                    }
//                    done = true;
                } else
                {
                    // handle buffer overflow
                    mvprintw(y - 2, x / 2 - 15, "Error: buffer overflow");
                }
                break;
            default:
                // handle other key presses
                break;
        }
        // consume newline character
        if (ch != KEY_DOWN && ch != KEY_UP)
        {
            getch();
        }
    }

    free(buffer);
    noecho();
    clear();
    refresh();
    delwin(register_win);
}

void* input_handler(void* arg) {
    int ch;
    struct dc_env *env;
    struct dc_error *err;
    int socket_fd;

    struct arg *args = (struct arg *) arg;
    env = args->env;
    err = args->error;
    socket_fd = args->socket_fd;

    bool quit = false;
    int input_idx = 0;
    char input_buffer[COLS - 2];
    time_t time_send = time(NULL);
    uint8_t send_time = time_send;
    char message[1024];
    char channel_name[30] = "comp4981 channel";
    char ETX[3] = "\x03";
    memset(input_buffer, 0, sizeof(input_buffer));
    draw_menu(menu_highlight);

    while (!quit) {
        ch = getch();

        pthread_mutex_lock(&mutex);

        if (menu_focused && menu_active) {
            // clear input window
            werase(input_win);
            box(input_win, 0, 0);
            wrefresh(input_win);

            switch (ch) {
                case KEY_UP:
                    if (menu_highlight > 0) {
                        menu_highlight--;
                    }
                    break;
                case KEY_DOWN:
                    if (menu_highlight < MENU_ITEMS - 1) {
                        menu_highlight++;
                    }
                    break;
                case KEY_ENTER:
                case '\n':
                case '\r':
                    handle_menu_selection(env, err, socket_fd, menu_highlight);
                    break;
                case '\t': // Press 'Tab' to switch focus between input and menu
                    menu_focused = !menu_focused;
                    break;
                default:
                    break;
            }

            draw_menu(menu_highlight);
        } else {
            // Enter message
            mvwprintw(input_win, 1, 1, "Enter message: ");
            wrefresh(input_win);

            switch (ch) {
                case KEY_ENTER:
                case '\n':
                case '\r':
                    snprintf(message, sizeof(message), "%s%s%s%s%s%s%hhu%s",
                             display_name, ETX, current_chat, ETX,
                             input_buffer, ETX, send_time, ETX);
                    wprintw(input_win, "%s", message);
                    send_create_message(env, err, socket_fd, message);
                    wprintw(chat_win, "%s %s %s", display_name, input_buffer, ctime(&time_send));
                    werase(input_win);
                    box(input_win, 0, 0);
                    wrefresh(input_win);
                    input_idx = 0;
                    memset(input_buffer, 0, sizeof(input_buffer));
                    mvwprintw(input_win, 1, 1, "Enter message: ");
                    wrefresh(input_win);
                    refresh();
                    break;
                case KEY_BACKSPACE:
                case KEY_DC:
                case 127: // Backspace key
                    if (input_idx > 0) {
                        input_idx--;
                        input_buffer[input_idx] = '\0';
                        werase(input_win);
                        box(input_win, 0, 0);
                        mvwprintw(input_win, 1, 16, "%s", input_buffer);
                        wrefresh(input_win);
                    }
                    break;
                case '\t': // Press 'Tab' to switch focus between input and menu
                    menu_focused = !menu_focused;
                    break;
                case KEY_UP:
                    break;
                case KEY_DOWN:
                    break;
                case KEY_LEFT:
                    break;
                case KEY_RIGHT:
                    break;
                default:
                    if (input_idx < COLS - 3) {
                        input_buffer[input_idx] = ch;
                        input_idx++;
                        mvwprintw(input_win, 1, 16, "%s", input_buffer);
                        wrefresh(input_win);
                    }
                    break;
            }
        }

        pthread_mutex_unlock(&mutex);

        // Sleep to prevent high CPU usage
        usleep(10000);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    stdscr = initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    struct dc_env *env1;
    struct dc_error *err1;
    int socket_fd1;
//    void *arg;

    err1 = dc_error_create(false);
    env1 = dc_env_create(err1, true, NULL);


    pthread_mutex_init(&mutex, NULL);
    pthread_t input_thread, message_thread;
//    pthread_create(&message_thread, NULL, message_handler, NULL);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_RED, COLOR_WHITE);
    struct sockaddr_in server_addr;
    char buffer[MAX_SIZE];
    bool run_client = true;

    if (argc < 2)
    {
        fprintf(stderr, "Server IP: %s <server_ip>\n", argv[0]);
        run_client = false;
    }

    uint16_t server_port;
    if (argc >= 3)
    {
        server_port = strtol(argv[2], NULL, 10);
        if(server_port == 0)
        {
            fprintf(stderr, "Bad Port\n");
        }
    }
    else
    {
        server_port = SERVER_PORT;
    }

    char buffer2[1024];
    ssize_t num_read = read(STDIN_FILENO, buffer2, sizeof(buffer2));
    if (num_read == -1) {
        perror("read failed");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Child process received: %.*s", (int)num_read, buffer2);

    socket_fd1 = socket(AF_INET, SOCK_STREAM, 0);
    struct arg arg1;
    arg1.error = err1;
    arg1.env = env1;
    arg1.socket_fd = socket_fd1;


    if (socket_fd1 < 0)
    {
        perror("Failed to create socket");
        run_client = false;
    }

    dc_memset(env1, &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    printf("Trying to connect to server %s: %d", argv[1], server_port);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("INET_PTON failed");
        run_client = false;
    }

    if (connect(socket_fd1, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect failed");
        run_client = false;
    }

    if (run_client) {
        fprintf(stderr, "Connected to server.\n");
        refresh();
        draw_login_win(env1, err1, socket_fd1);
//        init_windows();
        pthread_create(&input_thread, NULL, input_handler, &arg1);
    }

    free(env1);
    free(err1);
    pthread_join(input_thread, NULL);
//    pthread_join(message_thread, NULL);

    pthread_mutex_destroy(&mutex);
    close(socket_fd1);

    return EXIT_SUCCESS;
}

long get_response_code(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    uint32_t header;
    char body[MAX_SIZE];
    long response;
    ssize_t n;

    // receive header from server
    n = read(socket_fd, &header, sizeof(header));
    if (n < 0) {
        perror("error");
        exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
    }

    struct binary_header * binaryHeaderField = deserialize_header(header);

    printf("RECEIVED FROM SERVER");
    // print deserialized header
//    fprintf(stderr, "Version: %d\n", binaryHeaderField->version);
//    fprintf(stderr, "Type: %d\n", binaryHeaderField->type);
//    fprintf(stderr, "Object: %d\n", binaryHeaderField->object);
//    fprintf(stderr, "Body Size: %d\n", binaryHeaderField->body_size);

    // Read body and clear buffer
    read(socket_fd, &body, MAX_SIZE);

    // Response is 200\0x3name\0x3
    // get it the name and response code from the body
    char *response_code = strtok(body, "\x3");
    char *name = strtok(NULL, "\x3");

    // assign name to global variable that is not malloced yet
    display_name = malloc(strlen(name) + 1);
    strcpy(display_name, name);


    // convert response code to long
    response = strtol(response_code, NULL, 10);

    // print response code
    fprintf(stderr, "Response Code: %ld\n", response);

    // print name
    fprintf(stderr, "Name: %s\n", name);

    // free memory
    free(binaryHeaderField);
    dc_memset(env, body, 0, MAX_SIZE);


    return response;
}




void quit()
{
    endwin();
    exit(0);
}