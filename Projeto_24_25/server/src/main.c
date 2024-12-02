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

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(GSport);


    // Bind UDP socket
    if (bind(udp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("UDP socket bind failed");
        close(udp_socket);
        return EXIT_FAILURE;
    }

    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("TCP socket creation failed");
        close(udp_socket);
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(GSport);

    //TODO TCP após udp e paralelização
    // Bind TCP socket
    if (bind(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("TCP socket bind failed");
        close(udp_socket);
        close(tcp_socket);
        return EXIT_FAILURE;
    }

    printf("Game Server is up and running.\n");

    char buffer[1024];  // Buffer to store incoming messages
    struct sockaddr_in client_addr;  // Client address structure
    socklen_t client_addr_len = sizeof(client_addr);

    fd_set read_fds, temp_fds;
    FD_ZERO(&read_fds);
    int max_fd = udp_socket > tcp_socket ? udp_socket : tcp_socket;
    FD_SET(udp_socket, &read_fds);
    FD_SET(tcp_socket, &read_fds);

    // Start listening for TCP connections
    if (listen(tcp_socket, 5) < 0) {
        perror("TCP socket listen failed");
        close(udp_socket);
        close(tcp_socket);
        return EXIT_FAILURE;
    }

    //TODO Implementar sistema de queues

    while (1) {

        temp_fds = read_fds;

        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) < 0) {
            perror("Failed to select");
            continue;
        }

        if (FD_ISSET(udp_socket, &temp_fds)) {

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
        if (FD_ISSET(tcp_socket, &temp_fds)) {
            //TCP SERVER
            printf("TCP connection received\n");
            pid_t pid;
            pid = fork();
            if (pid < 0) {
                perror("Failed to fork");
                return EXIT_FAILURE; //TODO deal with this later
            }
            if (pid == 0) {
                sleep(1);
                int client_socket;
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                if ((client_socket = accept(tcp_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
                    perror("Failed to accept TCP connection");
                    return EXIT_FAILURE;
                }
                printf("Accepted TCP connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                struct timeval timeout;
                timeout.tv_sec = 5;
                timeout.tv_usec = 0;

                if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
                    fprintf(stderr, "ERROR: socket timeout creation was not sucessful\n");
                    close(client_socket);
                    exit(1);
                }

                memset(buffer, 0, sizeof(buffer));
                int n = read_tcp_socket(client_socket, buffer, 4); 
                if (n < 0) {
                    if (errno == EWOULDBLOCK || errno == EAGAIN) {
                        perror("Timeout");
                        close(client_socket);
                        exit(0);
                    }
                    perror("Failed to read from TCP socket");
                    close(client_socket);
                    exit(1);
                }
                buffer[n-1] = '\0';
                printf("[DEBUG] Received message: %s\n", buffer);
                if (verbose_mode) {
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                    printf("Received message from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
                    printf("Message: %s\n", buffer);
                }
                if (strcmp(buffer, "STR") == 0) {
                    handle_trials_request(client_socket);
                }
                else if (strcmp(buffer, "SSB") == 0) {
                    //handle_scoreboard_request(client_socket);
                }
                else {
                    //NOT SURE IF THIS IS THE RIGHT RESPONSE
                    send_tcp_response("ERR\n", client_socket);
                }
                close(client_socket);
                printf("Connection closed\n");
                exit(0); //Kill the child process
            }
        }
    }

    close(udp_socket);
    close(tcp_socket);

    return 0;
}