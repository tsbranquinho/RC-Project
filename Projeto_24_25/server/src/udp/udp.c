#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void send_udp_response(const char *message, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    printf("Sending UDP response: %s\n", message);
    pthread_mutex_lock(&fd_mutex);
    if (sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *)client_addr, client_addr_len) < 0) {
        perror("Failed to send UDP response");
    }
    pthread_mutex_unlock(&fd_mutex);
}