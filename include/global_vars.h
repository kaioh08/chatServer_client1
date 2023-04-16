//
// Created by Colin Lam on 2023-04-16.
//

#ifndef PROCESS_SERVER_GLOBAL_VARS_H
#define PROCESS_SERVER_GLOBAL_VARS_H

#include <pthread.h>

#define ETX "\3"

int response_buffer_updated;
char *current_channel;
char *display_name;
pthread_mutex_t response_buffer_mutex;
pthread_mutex_t socket_mutex;
pthread_mutex_t debug_file_mutex;

#endif //PROCESS_SERVER_GLOBAL_VARS_H
