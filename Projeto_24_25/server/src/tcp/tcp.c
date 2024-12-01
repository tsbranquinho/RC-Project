#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void send_tcp_response(const char *message, int tcp_socket) {
    int n, sum = 0;
    while (sum != strlen(message)) {
        n = write(tcp_socket, message, strlen(message));
        if (n == -1) {
            perror("Failed to send TCP message");
            return;
        }
        sum += n;
    }
}