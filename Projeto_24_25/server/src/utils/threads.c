#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int kill_threads = 0;
pthread_mutex_t kill_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t *mutex_plid(const char *plid) {

    pthread_mutex_t *plid_mutex = get_plid_mutex(plid);
    if (!plid_mutex) {
        fprintf(stderr, "Failed to acquire lock for player %s\n", plid);
        return NULL;
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
        pthread_mutex_lock(&kill_mutex);
        if (kill_threads) {
            pthread_mutex_unlock(&kill_mutex);
            pthread_mutex_unlock(&queue->lock);
            return (Task){0}; // Return an empty task if terminating
        }
        pthread_mutex_unlock(&kill_mutex);

        // Wait for tasks or termination signal
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
        char buffer[GLOBAL_BUFFER];
        struct sockaddr_in client_addr = task.client_addr;

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

        if (settings.verbose_mode) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        }

        if (strcmp(buffer, "STR") == 0) {
            handle_trials_request(client_socket);
        }
        else if (strcmp(buffer, "SSB") == 0) {
            handle_scoreboard_request(client_socket);
        }
        else {
            send_tcp_response("ERR\n", client_socket);
            close(client_socket);
        }
        printf("[TCP] Connection closed\n");

    } else {

        char *buffer = task.buffer;
        struct sockaddr_in client_addr = task.client_addr;
        socklen_t client_addr_len = task.addr_len;

        if (settings.verbose_mode) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
        }

        if (strncmp(buffer, "SNG", 3) == 0) {
            handle_start_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else if (strncmp(buffer, "TRY", 3) == 0) {
            handle_try_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else if (strncmp(buffer, "QUT", 3) == 0) {
            handle_quit_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else if (strncmp(buffer, "DBG", 3) == 0) {
            handle_debug_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else if (strncmp(buffer, "HNT", 3) == 0) {
            handle_hint_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else {
            send_udp_response("ERR\n", &client_addr, client_addr_len, settings.udp_socket);
        }
    }
}

void *worker_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&kill_mutex);
        if (kill_threads) {
            printf("Thread %ld exiting\n", pthread_self());
            pthread_mutex_unlock(&kill_mutex);
            break;
        }
        pthread_mutex_unlock(&kill_mutex);

        Task task = task_queue_pop(&task_queue);

        pthread_mutex_lock(&kill_mutex);
        if (kill_threads) {
            pthread_mutex_unlock(&kill_mutex);
            break;
        }
        pthread_mutex_unlock(&kill_mutex);

        handle_task(task);
    }
    return NULL;
}

void kill_sig(int signo) {
    pthread_mutex_lock(&kill_mutex);
    kill_threads = 1;
    pthread_mutex_unlock(&kill_mutex);

    pthread_cond_broadcast(&task_queue.cond);

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads terminated.\n");
}