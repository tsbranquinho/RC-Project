#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int connect_to_server(struct addrinfo **res) {
    struct addrinfo hints;
    int sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", GSport);

    if (getaddrinfo(GSIP, port_str, &hints, res) != 0) {
        perror("ERROR: Failed to resolve server address");
        return -1;
    }

    sockfd = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    if (sockfd == -1) {
        perror("ERROR: Failed to create socket");
        freeaddrinfo(*res);
        return -1;
    }

    if (connect(sockfd, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        perror("ERROR: Connect to server failed");
        close(sockfd);
        freeaddrinfo(*res);
        return -1;
    }

    return sockfd;
}

int send_tcp_message(int fd, const char *message) {
    size_t total_written = 0;
    size_t message_length = strlen(message);

    while (total_written < message_length) {
        ssize_t bytes_written = write(fd, message + total_written, message_length - total_written);
        if (bytes_written < 0) {
            perror("ERROR: Failed to send message");
            return -1;
        }
        total_written += bytes_written;
    }
    return 0;
}

int read_tcp_socket(int fd, char *buffer, size_t size) {
    memset(buffer, 0, size); // Initialize buffer
    size_t bytes_read = 0;

    while (size > 0) {
        ssize_t bytes = read(fd, buffer + bytes_read, size);
        printf("Bytes: %ld\n", bytes);
        if (bytes < 0) {
            //if (errno == ECONNRESET && buffer[bytes_read - 1] == '\n') {
            //    printf("END\n");
            //    break;
            //}
            perror("ERROR: Failed to read from socket");
            return -1;
        } else if (bytes == 0) {
            break;
        }
        bytes_read += bytes;
        size -= bytes;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the response
    return 0;
}