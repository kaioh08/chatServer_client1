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
#include "global_vars.h"

#define SERVER_PORT 5432
#define BUFFER_SIZE 1024
//#define MAX_SIZE 1024
//#define INPUT_HEIGHT 3
//#define MENU_WIDTH 30
//#define MAX_NAME_LENGTH 20
//#define MAX_PASSWORD_LENGTH 20
//#define MENU_ITEMS 5
//#define MAX_SIZE 1024

pthread_mutex_t mutex;
// Thread functions
//WINDOW *menu_win, *chat_win, *input_win, *login_win, *register_win;

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
    pthread_mutex_init(&response_buffer_mutex, NULL);
    pthread_mutex_init(&socket_mutex, NULL);
    pthread_mutex_init(&debug_file_mutex, NULL);
    pthread_t input_thread, message_thread;
    response_buffer_updated = 0;
    char *response_buffer = dc_malloc(env1, err1, BUFFER_SIZE*sizeof(char));
    struct binary_header_field *b_header = dc_malloc(env1, err1, sizeof(struct binary_header_field));
    display_name = dc_malloc(env1, err1, 20* sizeof(char));
    memset(display_name, '\0', 20* sizeof(char));
    current_channel = dc_malloc(env1, err1, 20* sizeof(char));
    memset(current_channel, '\0', 20* sizeof(char));

    curs_set(0);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_RED, COLOR_WHITE);
    struct sockaddr_in server_addr;
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

    FILE *file = dc_fopen(env1, err1, "debug_log.txt", "w");
    socket_fd1 = socket(AF_INET, SOCK_STREAM, 0);
    struct arg arg1;
    arg1.error = err1;
    arg1.env = env1;
    arg1.socket_fd = socket_fd1;
    arg1.response_buffer = response_buffer;
    arg1.b_header = b_header;
    arg1.debug_log_file = file;

    struct read_handler_args arg2;
    arg2.err = err1;
    arg2.env = env1;
    arg2.socket_fd = socket_fd1;
    arg2.response_buffer = response_buffer;
    arg2.b_header = b_header;
    arg2.debug_log_file = file;
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
        fprintf(file, "Running Client\n");
        pthread_create(&message_thread, NULL, read_message_handler, &arg2);
        draw_login_win(env1, err1, socket_fd1, file, response_buffer);
        pthread_create(&input_thread, NULL, input_handler, &arg1);
    }

    free(env1);
    free(err1);
    free(response_buffer);
    pthread_join(input_thread, NULL);
    pthread_join(message_thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&response_buffer_mutex);
    pthread_mutex_destroy(&socket_mutex);
    pthread_mutex_destroy(&debug_file_mutex);
    close(socket_fd1);

    return EXIT_SUCCESS;
}

