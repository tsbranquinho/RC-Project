#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int send_udp_skt(const char *message, char *response, int response_size, const char *server_ip, int server_port) {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    struct timeval timeout = {5, 0};

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Set socket option failed");
        close(sockfd);
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(sockfd);
        return -1;
    }

    if (sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("Send failed");
        close(sockfd);
        return -1;
    }

    if (recvfrom(sockfd, response, response_size, 0, (struct sockaddr *)&server_addr, &addr_len) < 0) {
        perror("Receive failed or timed out");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}