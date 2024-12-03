#include "../include/constants.h"
#include "../include/prototypes.h"
#include "../include/globals.h"

Player *hash_table[MAX_PLAYERS] = {NULL};
pthread_mutex_t lock_table_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t *lock_table_plid[MAX_LOCKS] = {NULL};
pthread_rwlock_t hash_table_lock = PTHREAD_RWLOCK_INITIALIZER;

TaskQueue task_queue;
int udp_socket, tcp_socket;
int verbose_mode = 0;

pthread_mutex_t *mutex_plid(const char *plid) {

    pthread_mutex_t *plid_mutex = get_plid_mutex(plid);
    if (!plid_mutex) {
        fprintf(stderr, "Failed to acquire lock for player %s\n", plid);
        return NULL; // Return NULL if the lock can't be acquired
    }


    pthread_mutex_lock(plid_mutex);


    return plid_mutex;
}

void mutex_unlock(pthread_mutex_t *plid_mutex) {
    if(pthread_mutex_unlock(plid_mutex) != 0) {
        perror("Error unlocking mutex");
    }
}

void task_queue_init(TaskQueue *queue) {
    queue->front = 0;
    queue->rear = -1;
    queue->count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond, NULL);
}

void task_queue_push(TaskQueue *queue, Task task) {
    pthread_mutex_lock(&queue->lock);

    while (queue->count == MAX_TASK_QUEUE) {
        pthread_cond_wait(&queue->cond, &queue->lock);
    }

    queue->rear = (queue->rear + 1) % MAX_TASK_QUEUE;
    queue->tasks[queue->rear] = task;
    queue->count++;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->lock);
}

Task task_queue_pop(TaskQueue *queue) {
    pthread_mutex_lock(&queue->lock);

    while (queue->count == 0) {
        pthread_cond_wait(&queue->cond, &queue->lock);
    }

    Task task = queue->tasks[queue->front];
    queue->front = (queue->front + 1) % MAX_TASK_QUEUE;
    queue->count--;

    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->lock);

    return task;
}

void handle_task(Task task) {
    if (task.is_tcp) {

        int client_socket = task.client_socket;
        char buffer[1024];
        struct timeval timeout = {5, 0};
        struct sockaddr_in client_addr = task.client_addr;

        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("Error setting socket timeout");
            close(client_socket);
            return;
        }

        memset(buffer, 0, sizeof(buffer));
        int n = read_tcp_socket(client_socket, buffer, 4); 
        if (n < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                perror("Timeout");
                close(client_socket);
                exit(0);
            }
            perror("Failed to read from TCP socket");
            close(client_socket);
            exit(1);
        }
        buffer[n-1] = '\0';
        printf("[DEBUG] Received message: %s\n", buffer);

        if (verbose_mode) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
            printf("Message: %s\n", buffer);
        }

        if (strcmp(buffer, "STR") == 0) {
            handle_trials_request(client_socket);
        }
        else if (strcmp(buffer, "SSB") == 0) {
            //handle_scoreboard_request(client_socket);
        }
        else {
            //NOT SURE IF THIS IS THE RIGHT RESPONSE
            send_tcp_response("ERR\n", client_socket);
        }

        close(client_socket);
        printf("[TCP] Connection closed\n");

    } else {

        char *buffer = task.buffer;
        struct sockaddr_in client_addr = task.client_addr;
        socklen_t client_addr_len = task.addr_len;

        if (verbose_mode) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
            printf("Message: %s\n", buffer);
        }

        if (strncmp(buffer, "SNG", 3) == 0) {
            handle_start_request(buffer, &client_addr, client_addr_len, udp_socket);
        } else if (strncmp(buffer, "TRY", 3) == 0) {
            handle_try_request(buffer, &client_addr, client_addr_len, udp_socket);
        } else if (strncmp(buffer, "QUT", 3) == 0) {
            handle_quit_request(buffer, &client_addr, client_addr_len, udp_socket);
        } else if (strncmp(buffer, "DBG", 3) == 0) {
            handle_debug_request(buffer, &client_addr, client_addr_len, udp_socket);
        } else {
            send_udp_response("ERR\n", &client_addr, client_addr_len, udp_socket);
        }
    }
}

void *worker_thread(void *arg) {
    while (1) {
        Task task = task_queue_pop(&task_queue);
        handle_task(task);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int GSport = DEFAULT_PORT;
    int opt;
    signal(SIGINT, sig_detected);

    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':
                GSport = atoi(optarg);
                if (GSport < 0 || GSport > MAX_PORT) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'v':
                verbose_mode = 1;
                break;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    printf("Starting Game Server on port: %d\n", GSport);

    task_queue_init(&task_queue);

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (udp_socket < 0 || tcp_socket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(GSport)
    };

    if (bind(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ||
        bind(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket bind failed");
        return EXIT_FAILURE;
    }

    if (listen(tcp_socket, 5) < 0) {
        perror("TCP listen failed");
        return EXIT_FAILURE;
    }

    printf("Game Server is running.\n");

    int num_threads = sysconf(_SC_NPROCESSORS_ONLN) * 2;
    if (num_threads <= 0) num_threads = 4;
    printf("Number of threads: %d\n", num_threads);

    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    fd_set read_fds, temp_fds;
    FD_ZERO(&read_fds);
    FD_SET(udp_socket, &read_fds);
    FD_SET(tcp_socket, &read_fds);
    int max_fd = udp_socket > tcp_socket ? udp_socket : tcp_socket;

    while (1) {
        temp_fds = read_fds;
        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) < 0) {
            perror("Select failed");
            continue;
        }

        if (FD_ISSET(udp_socket, &temp_fds)) {
            printf("udp\n");
            char buffer[1024];
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            ssize_t len = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0,
                                   (struct sockaddr *)&client_addr, &addr_len);
            if (len > 0) {
                buffer[len] = '\0';
                Task task = {.client_addr = client_addr, .addr_len = addr_len, .is_tcp = 0};
                strncpy(task.buffer, buffer, sizeof(task.buffer));
                task_queue_push(&task_queue, task);
            }
        }

        if (FD_ISSET(tcp_socket, &temp_fds)) {
            printf("here\n");
            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int client_socket = accept(tcp_socket, (struct sockaddr *)&client_addr, &addr_len);
            if (client_socket >= 0) {
                Task task = {.client_socket = client_socket, .client_addr = client_addr, .addr_len = addr_len, .is_tcp = 1};
                task_queue_push(&task_queue, task);
            }
        }
    }

    close(udp_socket);
    close(tcp_socket);
    return 0;
}

