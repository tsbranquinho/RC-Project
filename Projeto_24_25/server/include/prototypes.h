#ifndef __PROTOTYPES_H__
#define __PROTOTYPES_H__

#include "constants.h"

//connections.c
void get_arguments(int argc, char *argv[]);
void setup_server();
void create_tcp_socket();
void create_udp_socket();
void thread_configuration();
//socket_manager.c
int select_handler();
void udp_connection();
void tcp_connection();
//udp.c
void send_udp_response(const char *message, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
int udp_handler(char* buffer, struct sockaddr_in client_addr, socklen_t client_addr_len);
//tcp.c
void send_tcp_response(char *message, int tcp_socket);
int read_tcp_socket(int fd, char *buffer, size_t size);
int tcp_handler(char *buffer, int client_socket, struct sockaddr_in client_addr);
//start.c
int handle_start_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
void generate_random_key(char *key);
void set_verbose_start_message(char* request, const char* plid, int max_time);
//try.c
int handle_try_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
int check_try_err(const char *request, int n_args, char *aux_guess, int trial_num);
int check_try_nok(const char *plid, Player *player);
int check_try_etm(Player *player, char* response);
int check_try_inv(Player *player, int trial_num, char *guess);
int check_try_dup(Player *player, char *guess);
int check_try_ent(Player *player, char* response, pthread_mutex_t *plid_mutex);
int calculate_feedback(const char *guess, const char *secret, int *black, int *white);
void create_trial(Player *player, char *guess, int black, int white);
int write_try_to_file(Player *player, char *guess, int black, int white);
void set_verbose_try_message(char* request, const char* plid, int trial_num, const char* guess);
//quit.c
int handle_quit_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
int end_game(Player *player, pthread_mutex_t *plid_mutex);
int score_game(Player *player);
void set_verbose_quit_message(char* request, const char* plid);
//debug.c
int handle_debug_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
int valid_key(const char *key);
void set_verbose_debug_message(char *request, const char *plid, int max_time, const char *key);
//trials.c
int handle_trials_request(int tcp_socket, char *buffer);
int find_last_game(char *PLID, char *filename);
void set_verbose_trials_message(char *request, const char *plid);
//scoreboard.c
int handle_scoreboard_request(int tcp_socket, char *buffer);
void find_top_scores(char* buffer);
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
void clean_server();
void clean_game(Game *game);
//error.c
void delete_directory_contents(const char *path);
void cleanup_server();
void sig_detected(int sig);
//threads.c
pthread_mutex_t *mutex_plid(const char *plid);
void mutex_unlock(pthread_mutex_t *plid_mutex);
void handle_task(Task task);
void *worker_thread(void *arg);
void task_queue_init(TaskQueue *queue);
void task_queue_push(TaskQueue *queue, Task task);
Task task_queue_pop(TaskQueue *queue);
void sig_detected(int signo);
void kill_sig(int signo);
//usage.c
void usage(const char *progname);
//hint.c
int handle_hint_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket);
void set_verbose_hint_message(char *request, const char *plid, int hint);

#endif