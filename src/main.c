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

#define SERVER_PORT 5001
#define MAX_SIZE 1024
WINDOW *login_win;

void quit();
void draw_login_win(struct dc_env *env, struct dc_error *err, int socket_fd);

void quit()
{
    endwin();
    exit(0);
}

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
    echo();
    move(login_y, login_x + login_len);
    refresh();
    getstr(username);

    mvprintw(password_y, password_x, "Password: ");
    move(password_y, password_x + password_len);
    refresh();
    echo();
    getstr(password);

    strcat(buffer, username);
    strcat(buffer, ETX);
    strcat(buffer, password);
    strcat(buffer, ETX);
    buffer[strlen(buffer)] = '\0';


    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
    mvprintw(password_y + 2, x / 2, "  Register  ");

    // highlight login button
    attron(A_REVERSE);
    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
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
                    // highlight register button
                    attron(A_REVERSE);
                    mvprintw(password_y + 2, x / 2, "  Register  ");
                    attroff(A_REVERSE);
                    // unhighlight login button
                    mvprintw(password_y + 2, x / 2 - 10, "  Login  ");
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
                }
                break;
            case '\n':
                if (strlen(buffer) < MAX_SIZE)
                {
                    send_create_auth(env, err, socket_fd, buffer);
                    done = true;
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

    refresh();
    delwin(login_win);
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
            body[(binaryHeaderField->body_size)] = '\0';
            fprintf(stderr, "Body: %s\n", body);
        }
        fprintf(stderr, "Client Disconnected.\n");
    }

    free(env);
    free(err);
    close(socket_fd);

    return EXIT_SUCCESS;
}
