#ifndef __PROTOTYPES_H__
#define __PROTOTYPES_H__

#include "constants.h"

void usage(const char *progname);
//udp.c
void send_udp_response(const char *message, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
//tcp.c
void send_tcp_response(char *message, int tcp_socket);
int read_tcp_socket(int fd, char *buffer, size_t size);
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
int check_try_ent(Player *player, char* response, pthread_mutex_t *plid_mutex);
int calculate_feedback(const char *guess, const char *secret, int *black, int *white);
void create_trial(Player *player, char *guess, int black, int white);
int write_try_to_file(Player *player, char *guess, int black, int white);
//quit.c
void handle_quit_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
int end_game(Player *player, pthread_mutex_t *plid_mutex);
int score_game(Player *player);
//debug.c
void handle_debug_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
//player.c
pthread_mutex_t *get_plid_mutex(const char *plid);
void cleanup_plid_mutex(const char *plid);
Player *create_player(const char *plid);
Player *find_player(const char *plid);
void remove_player(const char *plid, pthread_mutex_t *plid_mutex);
unsigned int hash(const char *plid);
unsigned int hash_lock(const char *plid);
void insert_player(Player *player);
int valid_plid(const char *plid);
//error.c
void delete_directory_contents(const char *path);
void sig_detected(int sig);

void handle_trials_request(int tcp_socket);
int FindLastGame(char *PLID, char *filename);

void handle_scoreboard_request(int tcp_socket);
int FindTopScores(char* buffer);

//TODO tirar threads da main

pthread_mutex_t *mutex_plid(const char *plid);
void mutex_unlock(pthread_mutex_t *plid_mutex);
void handle_task(Task task);
void *worker_thread(void *arg);
void task_queue_init(TaskQueue *queue);
void task_queue_push(TaskQueue *queue, Task task);
Task task_queue_pop(TaskQueue *queue);
void sig_detected(int signo);
void usage(const char *progname);

void print_debug(int value); //TODO remove

void handle_hint_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);


#endif