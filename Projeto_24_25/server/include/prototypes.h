#ifndef __PROTOTYPES_H__
#define __PROTOTYPES_H__

#include "constants.h"

void usage(const char *progname);
void send_udp_response(const char *message, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
//start.c
void handle_start_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
void generate_random_key(char *key);
//try.c
void handle_try_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
int check_try_err(const char *request, int n_args, char *aux_guess, int *trial_num);
int check_try_nok(const char *plid, Player *player);
int check_try_etm(Player *player, char* response);
int check_try_inv(Player *player, int trial_num, char *guess);
int check_try_dup(Player *player, char *guess);
int check_try_ent(Player *player, char* response);
int calculate_feedback(const char *guess, const char *secret, int *black, int *white);
void create_trial(Player *player, char *guess, int black, int white);
int write_try_to_file(Player *player, char *guess, int black, int white);
//quit.c
void handle_quit_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
int end_game(Player *player);
int score_game(Player *player);
//debug.c
void handle_debug_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
//player.c
Player *create_player(const char *plid);
Player *find_player(const char *plid);
void remove_player(const char *plid);
unsigned int hash(const char *plid);
void insert_player(Player *player);
//error.c
void delete_directory_contents(const char *path);
void sig_detected(int sig);

#endif