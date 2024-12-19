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

    int opt, test=1;
    settings.verbose_mode = 0;
    settings.GSport = DEFAULT_PORT;
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    signal(SIGINT, sig_detected);

    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':
                settings.GSport = atoi(optarg);
                if (settings.GSport < 0 || settings.GSport > MAX_PORT) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'v':
                settings.verbose_mode = 1;
                break;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    struct stat st;

    if (stat("GAMES", &st) == -1) {
        if (mkdir("GAMES", 0777) == -1) {
            perror("Error creating GAMES directory");
            return EXIT_FAILURE;
        }
    }
    if (stat("SCORES", &st) == -1) {
        if (mkdir("SCORES", 0777) == -1) {
            perror("Error creating SCORES directory");
            return EXIT_FAILURE;
        }
    }

    printf("Starting Game Server on port: %d\n", settings.GSport);

    task_queue_init(&task_queue);

    settings.udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    settings.tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (settings.udp_socket < 0 || settings.tcp_socket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    if (setsockopt(settings.tcp_socket, SOL_SOCKET, SO_REUSEADDR, &test, sizeof(test)) < 0) {
        perror("Error setting socket timeout");
        //lock mutex pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;

        pthread_mutex_lock(&fd_mutex);
        close(settings.tcp_socket);
        pthread_mutex_unlock(&fd_mutex);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(settings.GSport)
    };

    if (bind(settings.udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ||
        bind(settings.tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket bind failed");
        return EXIT_FAILURE;
    }

    if (listen(settings.tcp_socket, 5) < 0) {
        perror("TCP listen failed");
        return EXIT_FAILURE;
    }

    printf("Game Server is running.\n");

    num_threads = sysconf(_SC_NPROCESSORS_ONLN) * 2;
    if (num_threads <= 0) num_threads = 1;
    printf("Number of threads: %d\n", num_threads);

    threads = malloc(sizeof(pthread_t) * num_threads);
    if (threads == NULL) {
        perror("Thread pool allocation failed");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    fd_set read_fds, temp_fds;
    FD_ZERO(&read_fds);
    FD_SET(settings.udp_socket, &read_fds);
    FD_SET(settings.tcp_socket, &read_fds);
    int max_fd = settings.udp_socket > settings.tcp_socket ? settings.udp_socket : settings.tcp_socket;


    while (1) {
        temp_fds = read_fds;
        if (select(max_fd + 1, &temp_fds, NULL, NULL, &timeout) < 0) {
            perror("Select failed");
            continue;
        }

        if (FD_ISSET(settings.udp_socket, &temp_fds)) {
            char buffer[1024];
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            ssize_t len = recvfrom(settings.udp_socket, buffer, sizeof(buffer) - 1, 0,
                                   (struct sockaddr *)&client_addr, &addr_len);
            printf("Received UDP request: %s\n", buffer);
            if (len > 0) {
                buffer[len] = '\0';
                Task task = {.client_addr = client_addr, .addr_len = addr_len, .is_tcp = 0};
                strncpy(task.buffer, buffer, sizeof(task.buffer));
                task_queue_push(&task_queue, task);
            }
        }

        if (FD_ISSET(settings.tcp_socket, &temp_fds)) {
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int client_socket = accept(settings.tcp_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (client_socket >= 0) {
                Task task = {.client_socket = client_socket, .client_addr = client_addr, .addr_len = addr_len, .is_tcp = 1};
                task_queue_push(&task_queue, task);
            }
        }
    }
    return 0;
}

