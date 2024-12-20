#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void send_udp_response(const char *message, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    pthread_mutex_lock(&fd_mutex);
    if (sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *)client_addr, client_addr_len) < 0) {
        perror("Failed to send UDP response");
    }
    pthread_mutex_unlock(&fd_mutex);
}

int udp_handler(char* buffer, struct sockaddr_in client_addr, socklen_t client_addr_len) {
    if (strncmp(buffer, "SNG", 3) == 0) {
        return handle_start_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
    } else if (strncmp(buffer, "TRY", 3) == 0) {
        return handle_try_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
    } else if (strncmp(buffer, "QUT", 3) == 0) {
        return handle_quit_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
    } else if (strncmp(buffer, "DBG", 3) == 0) {
        return handle_debug_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
    } else if (strncmp(buffer, "HNT", 3) == 0) {
        return handle_hint_request(buffer, &client_addr, client_addr_len, settings.udp_socket);
    } else {
        send_udp_response("ERR\n", &client_addr, client_addr_len, settings.udp_socket);
        return ERROR;
    }
}