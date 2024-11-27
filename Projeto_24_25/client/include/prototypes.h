#ifndef __PROTOTYPES_H__
#define __PROTOTYPES_H__
#include "constants.h"

void get_arguments(int argc, char *argv[]);
Command get_next_command(char *input);
void sig_detected(int sig);
int is_valid_ip(const char *ip);
void usage(const char *progname);
void start_game(const char *plid, unsigned int time);
void receive_start_msg(const char *response, const char *plid);
void send_show_trials_msg(int sockfd);
void receive_show_trials_msg(int fd);
//scoreboard.c
void send_show_scoreboard_msg(int sockfd);
void receive_show_scoreboard_msg(int fd);
//tcp.c
int connect_to_server(struct addrinfo **res);
int send_tcp_message(int fd, const char *message);
int read_tcp_socket(int fd, char *buffer, size_t size);
//udp.c
int send_udp_skt(const char *message, char *response, int response_size, const char *server_ip, int server_port);
//start.c
void start_game(const char *plid, unsigned int time);
void receive_start_msg(const char *response, const char *plid);
//try.c
void try_code(const char *code);
void receive_try_msg(const char *response);
//quit.c
void quit_game();
void receive_quit_msg(const char *response);
void end_game();
//exit.c
void exit_game();
//debug.c
void debug_game(const char *plid, unsigned int time, const char *code);
void receive_debug_msg(const char *plid, const char *response);


#endif