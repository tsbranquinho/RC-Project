#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void send_tcp_response(const char *message, int tcp_socket) {
    ssize_t n, sum = 0;
    while (sum != strlen(message)) {
        n = write(tcp_socket, message, strlen(message));
        if (n == -1) {
            perror("Failed to send TCP message");
            return;
        }
        sum += n;
    }
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

        // Stop if we detect a newline, indicating the end of the message
        if (buffer[bytes_read - 1] == '\n') {
            break;
        }
    }

    buffer[bytes_read] = '\0'; // Null-terminate the response
    return 0;
}