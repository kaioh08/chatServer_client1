#include "../include/processor_utility.h"
#include <arpa/inet.h>
//#include "../include/gui.h"
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

#define SERVER_PORT 5050
#define MAX_SIZE 1024
#define INPUT_HEIGHT 3
#define MENU_WIDTH 30
#define MENU_ITEMS 5
#define MAX_SIZE 1024
#define LOGIN_WIDTH 50
#define LOGIN_HEIGHT 30

#define REGISTER_WIDTH 40
#define REGISTER_HEIGHT 15

bool menu_focused = false;
int menu_highlight = 0;
bool menu_active = true;

pthread_mutex_t mutex;
WINDOW *menu_win, *chat_win, *input_win, *login_win, *register_win;

//void init_windows();
void init_menu();
void init_chat();
void init_input();
void* input_handler(void* arg);
void* message_handler(void* arg);
void draw_menu(int highlight);
void draw_register_window(struct dc_env *env, struct dc_error *err, int socket_fd);
void draw_login_win(struct dc_env *env, struct dc_error *err, int socket_fd);
void display_settings();
void quit();

long get_response_code(struct dc_env *env, struct dc_error *err, int socket_fd);

void draw_login_win(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    int x, y, login_x, login_y, password_x, password_y, login_len, password_len;
    char username[20], password[20];
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
    getstr(username);


    move(password_y, password_x + password_len);
    refresh();
    echo();
    getstr(password);



    if(strlen(username) > 20)
    {
        mvprintw(password_y + 6, x / 2 - 15, "Error: username too long");
        quit();
    }
    if(strlen(password) < 6)
    {
        sleep(3);
        mvprintw(password_y + 6, x / 2 - 15, "Error: password too short");
//        quit();
    }

    strcat(buffer, username);
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
//                    init_windows();
                    send_create_auth(env, err, socket_fd, buffer);
                    long response = get_response_code(env, err, socket_fd);

                    if (response != 201)
                    {
                        mvprintw(y - 2, x / 2 - 15, "Error: invalid credentials");
                        wrefresh(login_win);
                        sleep(3);
                        draw_login_win(env, err, socket_fd);
                        done = true;
                    } else {
                        mvprintw(y - 2, x / 2 - 15, "Success: logged in");
                        wrefresh(login_win);
//                        init_windows();
//                        init_menu();
//                        init_chat();
//                        init_input();
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


void draw_register_window(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    int x, y, username_x, username_y, password_x, password_y, displayname_x, displayname_y, username_len, password_len, displayname_len;
    char username[20], password[20], displayname[20];
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


    mvprintw(password_y, password_x, "Password: ");


    mvprintw(displayname_y, displayname_x, "Display Name: ");



    mvprintw(displayname_y + 2, x / 2 - 10, "  Register  ");
    mvprintw(displayname_y + 2, x / 2, "  Cancel  ");

    echo();
    move(username_y, username_x + username_len);
    refresh();
    getstr(username);

    move(password_y, password_x + password_len);
    refresh();
    echo();
    getstr(password);

    move(displayname_y, displayname_x + displayname_len);
    refresh();
    echo();
    getstr(displayname);

    if(strlen(username) > 20)
    {
        mvprintw(displayname_y + 6, x / 2 - 15, "Error: username too long");
        quit();
    }
    if(strlen(password) < 6)
    {
        mvprintw(displayname_y + 6, x / 2 - 15, "Error: password too short");
//        quit();
    }

    strcat(buffer, username);
    strcat(buffer, ETX);
    strcat(buffer, password);
    strcat(buffer, ETX);
    strcat(buffer, displayname);
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
                        mvprintw(displayname_y + 6, x / 2 - 15, "Error: username already exists");
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
                        done = true;
                        //return to login screen
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
    refresh();
    delwin(register_win);
}


int main(int argc, char *argv[])
{
    stdscr = initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_RED, COLOR_WHITE);
    struct dc_env *env;
    struct dc_error *err;
    int socket_fd;
    struct sockaddr_in server_addr;
    char buffer[MAX_SIZE];
    bool run_client = true;

    if (argc < 2)
    {
        fprintf(stderr, "Server IP: %s <server_ip>\n", argv[0]);
        run_client = false;
    }

    err = dc_error_create(true);
    env = dc_env_create(err, true, NULL);

    char buffer2[1024];
    ssize_t num_read = read(STDIN_FILENO, buffer2, sizeof(buffer2));
    if (num_read == -1) {
        perror("read failed");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Child process received: %.*s", (int)num_read, buffer2);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0)
    {
        perror("Failed to create socket");
        run_client = false;
    }

    dc_memset(env, &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    printf("Trying to connect to server %s: %d", argv[1], SERVER_PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        perror("INET_PTON failed");
        run_client = false;
    }

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect failed");
        run_client = false;
    }
    char login[6] = "login";
    char pwd[5] = "pwd";
    char username[9] = "username";
    char etx[5] = "0x03";

    char *body = dc_malloc(env, err, 1024);

    dc_strcpy(env, body, login);
    dc_strcat(env, body, etx);
    dc_strcat(env, body, pwd);
    dc_strcat(env, body, etx);
    dc_strcat(env, body, username);
    dc_strcat(env, body, etx);

    if (run_client) {
        fprintf(stderr, "Connected to server.\n");
        refresh();
        draw_login_win(env, err, socket_fd);

        while(fgets(buffer, MAX_SIZE, stdin) != NULL)
        {
            ssize_t n1 = send(socket_fd, buffer, dc_strlen(env, buffer), 0);

//            send_create_user(env, err, socket_fd, body);

            printf("Body: %s\n", body);
            if (n1 < 0)
            {
                perror("send");
                return EXIT_FAILURE;
            }

            fprintf(stderr, "Written to server\n");
            write(STDOUT_FILENO, buffer, n1);

            get_response_code(env, err, socket_fd);

        }
        fprintf(stderr, "Client Disconnected.\n");
    }

    free(env);
    free(err);
    close(socket_fd);

    return EXIT_SUCCESS;
}

long get_response_code(struct dc_env *env, struct dc_error *err, int socket_fd)
{
    uint32_t header;
    char body[MAX_SIZE];
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
    fprintf(stderr, "Version: %d\n", binaryHeaderField->version);
    fprintf(stderr, "Type: %d\n", binaryHeaderField->type);
    fprintf(stderr, "Object: %d\n", binaryHeaderField->object);
    fprintf(stderr, "Body Size: %d\n", binaryHeaderField->body_size);

    // Read body and clear buffer
    read(socket_fd, &body, MAX_SIZE);
//            body[(binaryHeaderField->body_size)] = '\0';
    fprintf(stderr, "Body: %s\n", body);

    // parse body until the delimeter "\0x3"
    long response;
    char *token = strtok(body, "\0x3");
    // convert the first token to an int
    response = dc_strtol(env, err, token, NULL, 10);
    return response;
}


void quit()
{
    endwin();
    exit(0);
}