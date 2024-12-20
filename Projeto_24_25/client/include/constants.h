#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include "../../common/common.h"

#define MAX_PORT 65535              // Maximum port number
#define ID_SIZE 6                   // Size of player ID
#define MAX_PLAYTIME 600            // Maximum playtime in seconds
#define MAX_COLORS 4                // Maximum number of colors in the colour key
#define MAX_COMMAND_SIZE 1024       // Symbolic constant for the maximum command size, way bigger than needed
#define TIME_SIZE 3                 // Size of the time
#define START_MSG_SIZE 15           // Size of the start message (4 (SNG + space) + 6 (PLID) + 1 (space) + 3 (TIME) + 1 (newline))
#define LENGTH -1                   // For error checking
#define COLOR -2                    // For error checking
#define SPACE -3                    // For error checking
#define STATUS 6                    // Size of the status
#define COMMAND 4                   // Size of the command

typedef enum {
    CMD_START,
    CMD_TRY,
    CMD_SHOW_TRIALS,
    CMD_SCOREBOARD,
    CMD_QUIT,
    CMD_EXIT,
    CMD_DEBUG,
    CMD_INVALID,
    CMD_SET,
    CMD_SLEEP,
    CMD_CLEAN,
    CMD_HINT,
    CMD_HELP
} Command;

void get_arguments(int argc, char *argv[]);
Command get_next_command(char *input);
void sig_detected(int sig);
int is_valid_ip(const char *ip);
void usage(const char *progname);
int send_udp_skt(const char *message, char *response, int response_size, const char *server_ip, int server_port);
void start_game(const char *plid, unsigned int time);
void receive_start_msg(const char *response, const char *plid);
void try_code(const char *code);
int send_tcp_message(int fd, const char *message);
int receive_tcp_message(int fd, char *response, int response_size);
void receive_try_msg(const char *response);
void send_show_trials_msg(int sockfd);
void receive_show_trials_msg(int fd);
void send_show_scoreboard_msg(int sockfd);
void receive_show_scoreboard_msg(int fd);
int connect_to_server(struct addrinfo **res);


void quit_game();
void receive_quit_msg(const char *response);
void exit_game();
void debug_game(const char *plid, unsigned int time, const char *code);
void receive_debug_msg(const char *plid, const char *response);
void end_game();

#endif
