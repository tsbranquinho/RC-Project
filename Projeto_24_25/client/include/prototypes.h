#ifndef __PROTOTYPES_H__
#define __PROTOTYPES_H__
#include "constants.h"

void get_arguments(int argc, char *argv[]);
int is_valid_ip(const char *ip);
void usage(const char *progname);
//command.c
Command get_next_command(char *input);
int handle_command(Command cmd, const char* input);
//tcp.c
int connect_to_server(struct addrinfo **res);
int send_tcp_message(int fd, const char *message);
int read_tcp_socket(int fd, char *buffer, size_t size);
//udp.c
int send_udp_skt(const char *message, char *response, int response_size, const char *server_ip, int server_port);
//trials.c
int handle_show_trials(const char *input);
void send_show_trials_msg(int sockfd);
void receive_show_trials_msg(int fd);
int get_word(char *word, char *response);
//scoreboard.c
int handle_show_scoreboard(const char *input);
void send_show_scoreboard_msg(int sockfd);
void receive_show_scoreboard_msg(int fd);
//start.c
int handle_start(const char *input);
void start_game(const char *plid, unsigned int time);
void receive_start_msg(const char *response, const char *plid);
//try.c
int handle_try(const char *input);
void try_code(const char *code);
void receive_try_msg(const char *response);
//quit.c
int handle_quit(const char *input);
void quit_game();
void receive_quit_msg(const char *response);
void end_game();
//exit.c
int handle_exit(const char *input);
void exit_game();
//debug.c
int handle_debug(const char *input);
void debug_game(const char *plid, unsigned int time, const char *code);
void receive_debug_msg(const char *plid, const char *response);
//error.c
int errorCurrentPlayer(const char *plid);
void sig_detected(int sig);
int invalid_command_format(Command cmd);
void invalid_player_id(const char *plid);
void invalid_playtime(unsigned int time);
void error_communicating_with_server(const char *msg);
void error_no_game(Command cmd);
void invalid_code(int mode);
//set.c
int handle_set(const char* input);
//sleep.c
int handle_sleep(const char* input);
//clean.c
int handle_clean(const char* input);
//hint.c
int handle_hint(const char *input);
//help.c
int handle_help(const char *input);


#endif