#include "../include/processor_utility.h"
#include "gui.h"
#include "menu_functions.h"
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
#include <ctype.h>
#include <pthread.h>

#define SERVER_PORT 5432
#define MAX_SIZE 1024
#define INPUT_HEIGHT 3
#define MENU_WIDTH 30
#define MAX_NAME_LENGTH 20
#define MAX_PASSWORD_LENGTH 20
#define MENU_ITEMS 5
#define MAX_SIZE 1024

pthread_mutex_t mutex;
// Thread functions
//WINDOW *menu_win, *chat_win, *input_win, *login_win, *register_win;
void* message_handler(void* arg);


void* message_handler(void* arg) {
    while (true) {
        // TODO: Implement message sending and receiving
        usleep(100000); // Sleep to prevent high CPU usage
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
