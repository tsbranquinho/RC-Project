#include "../include/constants.h"
#include "../include/prototypes.h"
#include "../include/globals.h"

Player *hash_table[MAX_PLAYERS] = {NULL};

int main(int argc, char *argv[]) {
    int GSport = DEFAULT_PORT;
    int verbose_mode = 0;
    int port_set = 0;
    int opt;
    signal(SIGINT, sig_detected);


    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':
                if (port_set) {
                    usage(argv[0]);
                    return EXIT_FAILURE;
                }
                GSport = atoi(optarg);
                if (GSport < 0 || GSport > MAX_PORT) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    return EXIT_FAILURE;
                }
                port_set = 1;
                break;

            case 'v':
                verbose_mode = 1;
                break;

            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    printf("Starting Game Server on port: %d\n", GSport);
    //copilot I need to know where the server is running
    

    //TODO isto tbm tem de ser feito
    if (verbose_mode) {
        printf("Verbose mode enabled\n");
    }

    int udp_socket, tcp_socket;

    if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP socket creation failed");
        return EXIT_FAILURE;
    }

    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP socket creation failed");
        close(udp_socket);
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(GSport);


    // Bind UDP socket
    if (bind(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP socket bind failed");
        close(udp_socket);
        close(tcp_socket);
        return EXIT_FAILURE;
    }

    /* TODO TCP após udp e paralelização
    // Bind TCP socket
    if (bind(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("TCP socket bind failed");
        close(udp_socket);
        close(tcp_socket);
        return EXIT_FAILURE;
    }

    // Start listening for TCP connections
    if (listen(tcp_socket, 5) < 0) {
        perror("TCP socket listen failed");
        close(udp_socket);
        close(tcp_socket);
        return EXIT_FAILURE;
    }
    */

    printf("Game Server is up and running.\n");

    char buffer[1024];  // Buffer to store incoming messages
    struct sockaddr_in client_addr;  // Client address structure
    socklen_t client_addr_len = sizeof(client_addr);

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t recv_len = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0,
                                    (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len < 0) {
            perror("Failed to receive UDP message");
            continue;
        }

        buffer[recv_len] = '\0';
        if (verbose_mode) { //TODO isto nao chega
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received message from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
            printf("Message: %s\n", buffer);
        }

        if (strncmp(buffer, "SNG", 3) == 0) {
            handle_start_request(buffer, &client_addr, client_addr_len, udp_socket);
        } else if (strncmp(buffer, "TRY", 3) == 0) {
            handle_try_request(buffer, &client_addr, client_addr_len, udp_socket);
        } else if (strncmp(buffer, "QUT", 3) == 0) {
            handle_quit_request(buffer, &client_addr, client_addr_len, udp_socket);
        } else if (strncmp(buffer, "DBG", 3) == 0) {
            handle_debug_request(buffer, &client_addr, client_addr_len, udp_socket);
        } else {
            send_udp_response("ERR\n", &client_addr, client_addr_len, udp_socket);
        }
    }

    close(udp_socket);
    close(tcp_socket);

    return 0;
}