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
int calculate_feedback(const char *guess, const char *secret, int *black, int *white);
//quit.c
void handle_quit_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
void handle_debug_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
//player.c
Player *create_player(const char *plid);
Player *find_player(const char *plid);
void remove_player(const char *plid);
unsigned int hash(const char *plid);
void insert_player(Player *player);

#endif