#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void send_tcp_response(char *message, int tcp_socket) {
    ssize_t n = strlen(message);
    char* pointer = message;
    printf("Trying to send TCP message: %s\n", message);
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
    close(tcp_socket);
}


int read_tcp_socket(int fd, char *buffer, size_t size) {
    memset(buffer, 0, size); // Initialize buffer
    size_t bytes_read = 0;

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

    buffer[bytes_read] = '\0'; // Null-terminate the response
    return 0;
}