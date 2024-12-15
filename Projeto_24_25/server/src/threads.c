#include "../include/constants.h"
#include "../include/prototypes.h"
#include "../include/globals.h"

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

        if (settings.verbose_mode) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
            printf("Message: %s\n", buffer);
        }

        if (strcmp(buffer, "STR") == 0) {
            handle_trials_request(client_socket);
        }
        else if (strcmp(buffer, "SSB") == 0) {
            handle_scoreboard_request(client_socket);
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

        if (settings.verbose_mode) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
            printf("Message: %s\n", buffer);
        }

        if (strncmp(buffer, "SNG", 3) == 0) {
            handle_start_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else if (strncmp(buffer, "TRY", 3) == 0) {
            handle_try_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else if (strncmp(buffer, "QUT", 3) == 0) {
            handle_quit_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else if (strncmp(buffer, "DBG", 3) == 0) {
            handle_debug_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
        } else {
            send_udp_response("ERR\n", &client_addr, client_addr_len, settings.udp_socket);
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