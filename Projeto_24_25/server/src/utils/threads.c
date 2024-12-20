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
        if (settings.verbose_mode) {
            perror("Failed to unlock mutex");
        }
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
        int status = tcp_handler(task.buffer, task.client_socket, task.client_addr);
        if (settings.verbose_mode) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &task.client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(task.client_addr.sin_port));
        }
    } else {
        int status = udp_handler(task.buffer, task.client_addr, task.addr_len);
        if (settings.verbose_mode) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &task.client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(task.client_addr.sin_port));
            if (status == ERROR) {
                printf("Something went wrong with the request: %s", task.buffer);
            } else {
                printf("%s\n", task.buffer);
            }
        }

    }
}

void *worker_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&kill_mutex);
        if (kill_threads) {
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

}