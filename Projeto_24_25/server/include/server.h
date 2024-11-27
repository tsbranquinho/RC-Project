#ifndef __GS_H__
#define __GS_H__

#include "../../utils/common.h"


#define MAX_PORT 65535
#define MAX_PLAYERS 100
#define BUFFER_SIZE 128
#define SECRET_TO_CODE 0
#define CODE_TO_SECRET 1

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


void usage(const char *progname);
void handle_start_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
void handle_try_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
void handle_quit_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
void handle_debug_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
void send_udp_response(const char *message, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
Player *create_player(const char *plid);
Player *find_player(const char *plid);
void remove_player(const char *plid);
void generate_random_key(char *key);
int calculate_feedback(const char *guess, const char *secret, int *black, int *white);
unsigned int hash(const char *plid);
void insert_player(Player *player);
void convert_code(char *temp, char *secret, int mode);

#endif