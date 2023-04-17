//
// Created by Vasily Shorin on 2023-04-14.
//

#include <dc_env/env.h>
#include <dc_error/error.h>
#include <ncurses.h>
#include <gui.h>
#include <dc_c/dc_stdlib.h>
#include <dc_c/dc_string.h>
#include <pthread.h>
#include <unistd.h>
#include "processor_utility.h"
#include "menu_functions.h"
#include "handling-response.h"

#define MAX_SIZE 1024
#define INPUT_HEIGHT 3
#define MENU_WIDTH 30
#define MENU_ITEMS 5
bool menu_focused = false;
int menu_highlight = 0;
bool menu_active = true;
pthread_mutex_t mutex;
WINDOW *menu_win, *chat_win, *input_win, *login_win, *register_win;

void draw_menu(WINDOW *win, int highlight, int number_of_items)
{
    //TODO: change quit to logout
    const char *choices[] = {
            "Create new chat",
            "Show list of active chats",
            "Show active users",
            "Settings",
            "Quit",
    };

    for (int i = 0; i < number_of_items; i++) {
        if (highlight == i) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, i + 1, 1, "%s", choices[i]);
        wattroff(win, A_REVERSE);
    }
    wrefresh(win);
}

void apply_highlight(WINDOW *win, int y, int x, const char *str) {
    wattron(win, A_REVERSE);
    mvwprintw(win, y, x, str);
    wattroff(win, A_REVERSE);
}

void remove_highlight(WINDOW *win, int y, int x, const char *str) {
    mvwprintw(win, y, x, str);
}

void draw_login_win(struct dc_env *env, struct dc_error *err, int socket_fd, FILE *file, char *response_buffer)
{
    size_t x, y, login_x, login_y, password_x, password_y;
    size_t login_len, password_len;
    char username_login[20], password[20];
    char *buffer = dc_malloc(env, err, sizeof(char) * MAX_SIZE);
    dc_memset(env, buffer, 0, sizeof(char) * MAX_SIZE);
    login_win = newwin(10, 40, (LINES - 10) / 2, (COLS - 40) / 2);
    clear();

    getmaxyx(stdscr, y, x);

    login_len = dc_strlen(env, "Login: ");
    login_x = x / 2 - login_len;
    login_y = y / 2;
    password_len = dc_strlen(env, "Password: ");
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

    if(dc_strlen(env, username_login) > 20)
    {
        mvprintw(password_y + 6, x / 2 - 15, "Error: username_login too long");
        quit();
    }
    if(dc_strlen(env, password) < 6)
    {
        sleep(3);
        mvprintw(password_y + 6, x / 2 - 15, "Error: password too short");
//        quit();
    }

    dc_strcat(env, buffer, username_login);
    dc_strcat(env,buffer, ETX);
    dc_strcat(env,buffer, password);
    dc_strcat(env,buffer, ETX);
    buffer[dc_strlen(env, buffer)] = '\0';

    // highlight login button
    attron(A_REVERSE);
    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
    attroff(A_REVERSE);

    //TODO: let user choose between login and register before entering credentials

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
                    // highlight register button
                    attron(A_REVERSE);
                    mvprintw(password_y + 2, x / 2, "  Register  ");
                    attroff(A_REVERSE);
                    // unhighlight login button
                    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
                    int newCh = getch();
                    if(newCh =='\n')
                    {
                        draw_register_window(env, err, socket_fd, file, response_buffer);
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
                if (dc_strlen(env, buffer) < MAX_SIZE)
                {
                    send_create_auth(env, err, socket_fd, buffer);
                    while(!response_buffer_updated);
                    struct arg options;
                    memset(&options, 0, sizeof(struct arg));
                    options.env = env;
                    options.error = err;
                    options.debug_log_file = file;
                    pthread_mutex_lock(&response_buffer_mutex);
                    int status = handle_create_auth_response(&options, response_buffer);
                    if (status != 200)
                    {
                        mvprintw(y - 2, x / 2 - 15, "Error: invalid credentials");
                        wrefresh(login_win);
                        sleep(3);
                        response_buffer_updated = 0;
                        memset(response_buffer, '\0', BUFSIZ*sizeof(char));
                        pthread_mutex_unlock(&response_buffer_mutex);
                        draw_login_win(env, err, socket_fd, file, response_buffer);
                        done = true;
                    }
                    else
                    {
                        mvprintw(y - 2, x / 2 - 15, "Success: logged in");
                        wrefresh(login_win);
//                        display_name = malloc(sizeof(char) * strlen(username_login) + 1);
//                        strcpy(display_name, username_login);
                        init_windows(MENU_ITEMS);
                        refresh();
                        done = true;
                    }
                    response_buffer_updated = 0;
                    memset(response_buffer, '\0', BUFSIZ*sizeof(char));
                    pthread_mutex_unlock(&response_buffer_mutex);
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


void init_windows(int number_of_items)
{
    init_menu(number_of_items);
    init_chat();
    init_input();
    refresh();
}

void init_menu(int number_of_items)
{
    menu_win = newwin(LINES - INPUT_HEIGHT, MENU_WIDTH, 0, 0);
    wbkgd(menu_win, COLOR_PAIR(1));
    box(menu_win, 0, 0);
    wrefresh(menu_win);
    draw_menu(menu_win, 1, number_of_items);
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

//TODO: change to handle logout
void quit(void)
{
    endwin();
    exit(0);
}

void draw_register_window(struct dc_env *env, struct dc_error *err, int socket_fd, FILE *file, char *response_buffer)
{
    int x, y, username_x, username_y, password_x, password_y, displayname_x, displayname_y, username_len, password_len, displayname_len;
    char username_register[20], displayname[20], password[20];
    char *buffer = malloc(sizeof(char) * MAX_SIZE);
    dc_memset(env, buffer, 0, sizeof(char) * MAX_SIZE);
    register_win = newwin(12, 40, (LINES - 12) / 2, (COLS - 40) / 2);
    clear();

    getmaxyx(stdscr, y, x);

    username_len = dc_strlen(env, "Username: ");
    username_x = x / 2 - username_len;
    username_y = y / 2 - 1;
    password_len = dc_strlen(env, "Password: ");
    password_x = x / 2 - password_len;
    password_y = username_y + 2;
    displayname_len = dc_strlen(env, "Display Name: ");
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

    if(dc_strlen(env,username_register) > 20)
    {
        mvprintw(displayname_y + 6, x / 2 - 15, "Error: display_name too long");
        quit();
    }
    if(dc_strlen(env, password) < 6)
    {
        mvprintw(displayname_y + 6, x / 2 - 15, "Error: password too short");
//        quit();
    }

    dc_strcat(env, buffer, username_register);
    dc_strcat(env, buffer, ETX);
    dc_strcat(env, buffer, displayname);
    dc_strcat(env, buffer, ETX);
    dc_strcat(env, buffer, password);
    dc_strcat(env, buffer, ETX);

    buffer[dc_strlen(env, buffer)] = '\0';


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
                if (dc_strlen(env, buffer) < MAX_SIZE)
                {
//                    init_windows();
                    send_create_user(env, err, socket_fd, buffer);
                    while(!response_buffer_updated);
                    struct arg options;
                    memset(&options, 0, sizeof(struct arg));
                    options.env = env;
                    options.error = err;
                    options.debug_log_file = file;
                    pthread_mutex_lock(&response_buffer_mutex);
                    int status = handle_create_user_response(&options, response_buffer);
                    if (status != 201)
                    {
                        mvprintw(displayname_y + 6, x / 2 - 15, "Error: display_name already exists");
                        wrefresh(register_win);
                        sleep(2);
                        response_buffer_updated = 0;
                        memset(response_buffer, '\0', BUFSIZ*sizeof(char));
                        pthread_mutex_unlock(&response_buffer_mutex);
                        draw_register_window(env, err, socket_fd, file, response_buffer);
                        refresh();
                    }
                    else
                    {
                        mvprintw(displayname_y + 6, x / 2 - 15, "Registration successful");
                        wrefresh(register_win);
                        sleep(1);
                        response_buffer_updated = 0;
                        memset(response_buffer, '\0', BUFSIZ*sizeof(char));
                        pthread_mutex_unlock(&response_buffer_mutex);
                        draw_login_win(env, err, socket_fd, file, response_buffer);
                        refresh();
                        done=true;
                    }
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

void clean_up(void)
{
    touchwin(menu_win);
    wrefresh(menu_win);
    touchwin(chat_win);
    wrefresh(chat_win);
    touchwin(input_win);
    wrefresh(input_win);
}

void* input_handler(void* arg) {
    int ch;
    struct dc_env *env;
    struct dc_error *err;
    int socket_fd;
    char *response_buffer;
    struct binary_header_field *b_header;
    FILE *file;

    struct arg *args = (struct arg *) arg;
    env = args->env;
    err = args->error;
    socket_fd = args->socket_fd;
    response_buffer = args->response_buffer;
    b_header = args->b_header;
    file = args->debug_log_file;

    fprintf(file, "input_handler started\n");

    bool quit = false;
    int input_idx = 0;
    char input_buffer[COLS - 2];
    time_t time_send = time(NULL);
//    uint8_t send_time = time_send;
    char message[1024];
    dc_memset(env, input_buffer, 0, sizeof(input_buffer));
    draw_menu(menu_win, menu_highlight, MENU_ITEMS);
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

            draw_menu(menu_win, menu_highlight, MENU_ITEMS);
        } else {
            // Enter message
            mvwprintw(input_win, 1, 1, "Enter message: ");
            wrefresh(input_win);

            switch (ch) {
                case KEY_ENTER:
                case '\n':
                case '\r':
                    if(input_buffer[0] == '/')
                    {
                        command_wrapper(env, err, file, socket_fd, input_buffer, response_buffer);
                    }
                    else
                    {

                        snprintf(message, sizeof(message), "%s%s%s%s%s%s%ld%s",
                                 display_name, ETX, current_channel, ETX,
                                 input_buffer, ETX, time_send, ETX);
                        wprintw(input_win, "%s", message);
                        send_create_message(env, err, socket_fd, message);
                        wprintw(chat_win, "%s %s %s", display_name, input_buffer, ctime(&time_send));
                        werase(input_win);
                        box(input_win, 0, 0);
                        wrefresh(input_win);
                        input_idx = 0;
                        dc_memset(env, input_buffer, 0, sizeof(input_buffer));
                        mvwprintw(input_win, 1, 1, "Enter message: ");
                        wrefresh(input_win);
                        refresh();
                    }
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
        if(response_buffer_updated)
        {
            pthread_mutex_lock(&response_buffer_mutex);
            fprintf(file, "Input Handler Got Sth:\nversion: %d\ntype: %d\nobject: %hhu\nbody size: %d\nbody: %s\n",
                    b_header->version, b_header->type, b_header->object, b_header->body_size, response_buffer);
            response_handler_wrapper(env, err, args, b_header, response_buffer);
            fflush(file);
            setbuf(file, NULL);
            response_buffer_updated = 0;
            memset(response_buffer, '\0', BUFSIZ*sizeof(char));
            pthread_mutex_unlock(&response_buffer_mutex);
        }

        pthread_mutex_unlock(&mutex);

        // Sleep to prevent high CPU usage
        usleep(10000);
    }
    return NULL;
}

void command_wrapper(struct dc_env *env, struct dc_error *err, FILE *file, int socket_fd, char *command, char *response_buffer)
{
    char *operation;
    char *the_rest;

    operation = strtok_r(command, " ", &the_rest);
    if(dc_strcmp(env, operation, "/join") == 0)
    {
        write_simple_debug_msg(file, "calling join_channel_wrapper...\n");
        join_channel_wrapper(env, err, file, socket_fd, the_rest, response_buffer);
    }
    else
    {

    }
}

void join_channel_wrapper(struct dc_env *env, struct dc_error *err, FILE *file, int socket_fd, char *channel_name, char *response_buffer)
{
    //Channel name etx 0 etx 0 etx 1 etx 1 etx display-name etx 0 etx 0 etx
    char *buffer = dc_malloc(env, err, BUFSIZ* sizeof(char));
    //send update_channel dispatch containing username and channel name
    sprintf(buffer, "%s%s%d%s%d%s%d%s%d%s%s%s%d%s%d%s",
            channel_name, ETX, 0, ETX, 0, ETX, 1, ETX, 1, ETX, display_name, ETX, 0, ETX, 0, ETX);
    send_update_channel(env, err, socket_fd, buffer);
    while(!response_buffer_updated);
    //call handle_update_channel
    struct arg options;
    memset(&options, 0, sizeof(struct arg));
    options.env = env;
    options.error = err;
    options.debug_log_file = file;
    pthread_mutex_lock(&response_buffer_mutex);
    write_simple_debug_msg(file, "calling handle_update_channel_response...\n");
    int status = handle_update_channel_response(&options, response_buffer);
    if(status == ALL_GOOD)
    {
        //zero out values in current_channel, strcpy chanel_name into current_channel
        memset(current_channel, '\0', 20* sizeof(char));
        strcpy(current_channel, channel_name);
    }
    else
    {
        //TODO: write in the window that the join failed
        write_simple_debug_msg(file, "join channel failed\n");
    }
    pthread_mutex_unlock(&response_buffer_mutex);
    free(buffer);
}

