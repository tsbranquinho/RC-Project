#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void send_tcp_response(char *message, int tcp_socket) {
    ssize_t n = strlen(message);
    char* pointer = message;
    printf("Sending message: %s\n", message);

    pthread_mutex_lock(&fd_mutex);
    while (n > 0) {
        ssize_t bytes = write(tcp_socket, pointer, n);
        if (bytes < 0) {
            perror("ERROR: Failed to send message");
            return;
        }
        n -= bytes;
        pointer += bytes;
    }
    shutdown(tcp_socket, SHUT_WR); // Indica que o servidor terminou de enviar dados
    pthread_mutex_unlock(&fd_mutex);
}


int read_tcp_socket(int fd, char *buffer, size_t size) {
    memset(buffer, 0, size); // Initialize buffer
    size_t bytes_read = 0;

    pthread_mutex_lock(&fd_mutex);
    while (bytes_read < size - 1) {
        ssize_t n = read(fd, buffer + bytes_read, size - bytes_read - 1);
        if (n == 0) {
            // Connection closed by the peer
            break;
        } else if (n < 0) {
            perror("ERROR: Failed to read from socket");
            return -1;
        }
        bytes_read += n;

    }
    pthread_mutex_unlock(&fd_mutex);
    buffer[bytes_read] = '\0'; // Null-terminate the response
    return 0;
}

int tcp_handler(char *buffer, int client_socket, struct sockaddr_in client_addr) {
    memset(buffer, 0, SMALL_BUFFER);
    int n = read_tcp_socket(client_socket, buffer, 4); 
    if (n < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            perror("Timeout");
            close(client_socket);
            return ERROR;
        }
        perror("Failed to read from TCP socket");
        close(client_socket);
        return ERROR;
    }

    if (strcmp(buffer, "STR") == 0) {
        handle_trials_request(client_socket);
    }
    else if (strcmp(buffer, "SSB") == 0) {
        handle_scoreboard_request(client_socket);
    }
    else {
        send_tcp_response("ERR\n", client_socket);
    }
    close(client_socket);
    return SUCCESS;
}