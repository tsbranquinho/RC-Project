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
#include "../../common/common.h"


#define MAX_PORT 65535
#define MAX_PLAYERS 100
#define BUFFER_SIZE 128

typedef struct Trials {
    char guess[MAX_COLORS + 1];
    int black;
    int white;
    struct Trials *prev;
} Trials;

typedef struct Game {
    char secret_key[MAX_COLORS + 1];
    int trial_count;
    Trials *trial;
    time_t start_time;
    int max_time;
} Game;

typedef struct Player {
    char plid[ID_SIZE + 1];
    int is_playing;
    Game *current_game;
    struct Player *next;
} Player;

#endif