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


#define MAX_SIZE 1024
#define INPUT_HEIGHT 3
#define MENU_WIDTH 30
#define MENU_ITEMS 5
char *display_name;
char *current_chat;

pthread_mutex_t mutex;
WINDOW *menu_win, *chat_win, *input_win, *login_win, *register_win;

void draw_menu(WINDOW *win, int highlight, int number_of_items)
{
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

void draw_login_win(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    size_t x, y, login_x, login_y, password_x, password_y;
    size_t login_len, password_len;
    char username_login[20], password[20];
    char ETX[3] = "\x03";
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
    long response;
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
                if (dc_strlen(env, buffer) < MAX_SIZE)
                {
                    send_create_auth(env, err, socket_fd, buffer);
                    response = get_response_code(env, err, socket_fd);
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
                        init_windows(MENU_ITEMS);
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

void quit(void)
{
    endwin();
    exit(0);
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
    char *response_code = dc_strtok(env,body, "\x3");
    char *name = dc_strtok(env, NULL, "\x3");

    // assign name to global variable that is not malloced yet
    display_name = dc_malloc(env, err, dc_strlen(env, name) + 1);
    dc_strcpy(env, display_name, name);


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

void draw_register_window(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    int x, y, username_x, username_y, password_x, password_y, displayname_x, displayname_y, username_len, password_len, displayname_len;
    char username_register[20], displayname[20], password[20];
    char ETX[3] = "\x03";
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
