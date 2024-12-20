#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>  // For socket functions
#include <sys/socket.h>  // For socket functions
#include <sys/types.h>  // For data types
#include <netinet/in.h>  // For sockaddr_in
#include <time.h>  // For time functions
#include <signal.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <pthread.h>
#include "../../common/common.h"


#define MAX_PORT 65535
#define MAX_PLAYERS 128
#define MAX_LOCKS 128
#define GAME_FILE_NAME_SIZE 16
#define DEBUG 1
#define PLAY 2
#define MAX_TASK_QUEUE 512
#define START_MSG_SIZE 15
#define TRY_MSG_SIZE 21
#define DEBUG_MSG_SIZE 23
#define TRIALS_MSG_SIZE 8
#define QUIT_MSG_SIZE 11

typedef struct Trials {
    char guess[MAX_COLORS + 1];
    int black;
    int white;
    struct Trials *prev;
} Trials;

typedef struct Game {
    char secret_key[MAX_COLORS + 1];
    int trial_count;
    int hint_count;
    int mode;
    Trials *trial;
    time_t start_time;
    time_t last_time;
    int max_time;
    char end_status;
    char filename[GAME_FILE_NAME_SIZE+6]; // 6 for "GAMES/"
} Game;

typedef struct Player {
    char plid[ID_SIZE + 1];
    Game *current_game;
    struct Player *next;
    int fd;
} Player;

typedef struct {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    int is_tcp;
    char buffer[GLOBAL_BUFFER];
} Task;

typedef struct {
    Task tasks[MAX_TASK_QUEUE];
    int front;
    int rear;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} TaskQueue;

typedef struct {
    int verbose_mode;
    int udp_socket;
    int tcp_socket;
    int GSport;
    struct timeval timeout;
    fd_set read_fds;
    fd_set temp_fds;
} set;


#endif