#include "../include/constants.h"
#include "../include/prototypes.h"
#include "../include/globals.h"

Player *hash_table[MAX_PLAYERS] = {NULL};
pthread_mutex_t lock_table_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *lock_table_plid[MAX_LOCKS] = {NULL};
pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t hash_table_lock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t scoreboard_lock = PTHREAD_RWLOCK_INITIALIZER;
int num_threads = 0;
pthread_t *threads = NULL;
TaskQueue task_queue;
set settings = {0};


int main(int argc, char *argv[]) {

    signal(SIGINT, sig_detected);

    get_arguments(argc, argv);
    
    setup_server();

    while (1) {
        // Handle incoming requests
        if (select_handler() < 0) {
            continue;
        }

        // Check for UDP connection
        if (FD_ISSET(settings.udp_socket, &settings.temp_fds)) {
            udp_connection();
        }

        // Check for TCP connection
        if (FD_ISSET(settings.tcp_socket, &settings.temp_fds)) {
            tcp_connection();
        }
    }
    return 0;
}

