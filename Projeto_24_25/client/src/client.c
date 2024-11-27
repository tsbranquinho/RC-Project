#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "../include/client.h"
#include "../../utils/common.h"


//Global variables, they will never change.
char *GSIP = DEFAULT_IP;                 // Default IP
int GSport = DEFAULT_PORT;               // Default port
char plidCurr[ID_SIZE + 1];              // Current player ID
int currPlayer = 0;                      // Flag to check if a player is playing or not
int currTries = 0;                       // Number of tries of the current player (it starts with the "1st try")

void resolve_hostname(const char *hostname) {
    struct addrinfo hints, *res;
    char ipstr[INET_ADDRSTRLEN]; // Buffer for IPv4 string representation

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4 only
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
        fprintf(stderr, "Error: Unable to resolve hostname '%s'\n", hostname);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));

    GSIP = strdup(ipstr); // Save the resolved IP address
    freeaddrinfo(res);
}

void get_arguments(int argc, char *argv[]) {
    int opt, ip_set = 0, port_set = 0;
    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
            case 'n':
                if (ip_set) {
                    usage(argv[0]);
                }
                resolve_hostname(optarg); // Resolve the hostname to an IP
                ip_set = 1;
                break;

            case 'p':
                if (port_set) {
                    usage(argv[0]);
                }
                if (!is_number(optarg)) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                GSport = atoi(optarg);
                if (GSport < 0 || GSport > MAX_PORT) {
                    fprintf(stderr, "Error: Invalid port number\n");
                    exit(EXIT_FAILURE);
                }
                port_set = 1;
                break;

            default:
                usage(argv[0]);
        }
    }

    if (!ip_set || !port_set) {
        usage(argv[0]);
    }

    printf("Game Server IP: %s\n", GSIP);
    printf("Game Server Port: %d\n", GSport);
}

Command get_next_command(char *input) {
    if (strncmp(input, "start", 5) == 0) return CMD_START;
    if (strncmp(input, "try", 3) == 0) return CMD_TRY;
    if (strncmp(input, "st", 2) == 0 || strncmp(input, "show_trials", 11) == 0) return CMD_SHOW_TRIALS;
    if (strncmp(input, "sb", 2) == 0 || strncmp(input, "scoreboard", 10) == 0) return CMD_SCOREBOARD;
    if (strncmp(input, "quit", 4) == 0) return CMD_QUIT;
    if (strncmp(input, "exit", 4) == 0) return CMD_EXIT;
    if (strncmp(input, "debug", 5) == 0) return CMD_DEBUG;
    return CMD_INVALID;
}

void usage(const char *progname) {
    fprintf(stderr, "Usage: %s [-n GSIP] [-p GSport]\n", progname);
    fprintf(stderr, "  -n GSIP    Specify the IP address of the game server (optional).\n");
    fprintf(stderr, "  -p GSport  Specify the port number of the game server (optional).\n");
}

int errorCurrentPlayer(const char *plid){
    if(currPlayer && strcmp(plidCurr, plid) != 0){
        printf("There is already a player playing. Please quit the current game to start a new one.\n");
        return 1;
    }
    return 0;
}

int is_valid_ip(const char *ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}

void sig_detected(int sig) {
    if (currPlayer) {
        printf("Game interrupted. Exiting...\n");
        //quit_game(); //TODO só no final podemos tirar isto senão semrpe que fizermos ctrl c o server tem estar ligado e o crlh
        exit(0);
    } else {
        printf("Exiting...\n");
        exit(0);
        //exit_game();    //TODO só no final podemos tirar isto senão semrpe que fizermos ctrl c o server tem estar ligado e o crlh
    }
}

int main(int argc, char *argv[]) {

    get_arguments(argc, argv);

    signal(SIGINT, sig_detected);

    int sockfd;
    struct addrinfo *res;

    while (1) {
        printf("Enter command: \n");
        char input[MAX_COMMAND_SIZE];
        memset(input, 0, sizeof(input));
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            fprintf(stderr, "Error reading command.\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        Command cmd = get_next_command(input);
        
        switch (cmd) {
            case CMD_START: {
                char plid[MAX_COMMAND_SIZE];
                memset(plid, 0, sizeof(plid));
                int time;
                if (sscanf(input, "start %s %d", plid, &time) != 2 ) {
                    fprintf(stderr, "Invalid command.\n");
                    break;
                }

                if(errorCurrentPlayer(plid)){
                    break;
                }

                start_game(plid, time);
                break;
            }

            case CMD_TRY: {
                char code[8];
                memset(code, 0, sizeof(code));
                if (sscanf(input, "try %[^\n]", code) != 1) {
                    fprintf(stderr, "Invalid 'try' command format. Format: try C1 C2 C3 C4\n");
                    break;
                }
                try_code(code);
                break;
            }

            case CMD_SHOW_TRIALS:

                sockfd = connect_to_server(&res);
                if (sockfd < 0) continue;

                send_show_trials_msg(sockfd);

                close(sockfd);
                freeaddrinfo(res);
                break;

            case CMD_SCOREBOARD:

                sockfd = connect_to_server(&res);
                if (sockfd < 0) continue;

                send_show_scoreboard_msg(sockfd);

                close(sockfd);
                freeaddrinfo(res);
                break;

            case CMD_QUIT:

                if (sscanf(input, "quit") != 0) {
                    fprintf(stderr, "Invalid 'quit' command format.\n");
                    break;
                }

                quit_game();
                break;

            case CMD_EXIT:

                if (sscanf(input, "exit") != 0) {
                    fprintf(stderr, "Invalid 'exit' command format.\n");
                    break;
                }

                exit_game();
                return 0;

            case CMD_DEBUG: {

                char plid[7];
                unsigned int max_playtime;
                char code[MAX_COMMAND_SIZE];
                memset(plid, 0, sizeof(plid));
                memset(code, 0, sizeof(code));

                if (sscanf(input, "debug %s %u %s", plid, &max_playtime, code) != 3) {
                    fprintf(stderr, "Invalid 'debug' command format.\n");
                    break;
                }

                if(errorCurrentPlayer(plid)){
                    break;
                }

                debug_game(plid, max_playtime, code);
                break;
            }

            default:
                fprintf(stderr, "Invalid command.\n");
                break;
        }
    }
    return 0;
}

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

void start_game(const char *plid, unsigned int time) {

    if (strlen(plid) != ID_SIZE || !is_number(plid)) {
        fprintf(stderr, "Invalid player ID, must be 6 numerical digits.\n");
        return;
    }
    if (time < 0 || time > MAX_PLAYTIME) {
        fprintf(stderr, "Invalid playtime, must be between 0 and 600 seconds.\n");
        return;
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "SNG %s %03d\n", plid, time);

    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    printf("[DEBUG] Sending start game request: %s", message); // Debug log

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        fprintf(stderr, "Error communicating with the server.\n");
        return;
    }

    printf("[DEBUG] Received response: %s", response); // Debug log
    receive_start_msg(response, plid);
}

void receive_start_msg(const char *response, const char *plid) {
    char status[BUFFER_SIZE];
    memset(status, 0, sizeof(status));
    if (sscanf(response, "RSG %s", status) != 1) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
    }

    if (strcmp(status, "OK") == 0) {
        printf("Game started successfully! You can begin playing.\n");
        strcpy(plidCurr, plid);
        currTries = 0;
        currPlayer = 1;
    } else if (strcmp(status, "NOK") == 0) {
        printf("Game not started: an ongoing game exists for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error starting game. Please check your inputs or try again later.\n");
    } else {
        fprintf(stderr, "Unexpected server response: %s\n", status);
    }
}

void try_code(const char *code) {
    char trimmed_code[2*MAX_COLORS];
    printf("Code: %s\n", code);
    memset(trimmed_code, 0, sizeof(trimmed_code));
    snprintf(trimmed_code, sizeof(trimmed_code), "%s", code);
    trimmed_code[2*MAX_COLORS-1] = '\0';
    if (currPlayer == 0) {
        printf("No game started. Please start a game before trying a code.\n");
        return;
    }

    if (strlen(trimmed_code) != 2*MAX_COLORS - 1) {
        fprintf(stderr, "Invalid code length. It must be exactly 7 characters.\n");
        return;
    }

    for (int i = 0; i < 2*MAX_COLORS; i += 2) {
        printf("trimmed_code[%d]: %c\n", i, trimmed_code[i]);
        printf("trimmed_code[%d]: %c\n", i+1, trimmed_code[i+1]);
        if (strchr(COLOR_OPTIONS, trimmed_code[i]) == NULL) {
            fprintf(stderr, "Invalid color code. Use only R, G, B, Y, O, or P.\n");
            return;
        }
        if (trimmed_code[i+1] != ' ' && trimmed_code[i+1] != '\0') {
            fprintf(stderr, "Invalid code format. Use spacing.\n");
            return;
        }
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "TRY %s %s %d\n",plidCurr, trimmed_code, currTries+1);

    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    printf("[DEBUG] Sending try request: %s", message);

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        fprintf(stderr, "Error communicating with the server.\n");
        return;
    }

    printf("[DEBUG] Received response: %s", response);
    receive_try_msg(response);
}

void receive_try_msg(const char *response) {
    char status[BUFFER_SIZE];
    int black = 0, white = 0;
    char code[2 * MAX_COLORS];
    int tries;
    memset(status, 0, sizeof(status));
    memset(code, 0, sizeof(code));

    if (sscanf(response, "RTR %s %d %d %d %[^\n]", status, &tries, &black, &white, code) < 1) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
    }
    code[2*MAX_COLORS] = '\0';
    printf("check\n");
    printf("status: %s\n", status);

    //TODO: verificar para cada resposta de status
    //    : verificar se o code está bem mandado

    if (strcmp(status, "OK") == 0) {
        currTries = tries;
        printf("Tries: %d, Black: %d, White: %d\n", currTries, black, white);

        if (black == MAX_COLORS) {
            printf("Congratulations! You've cracked the secret code.\n");
        }
    } else if (strcmp(status, "DUP") == 0) {
        printf("Duplicate code entered. Try a different combination.\n");
    } else if (strcmp(status, "ENT") == 0) {
        printf("No more attempts left. You lose! The secret code was: %s\n", code);
    } else if (strcmp(status, "ETM") == 0) {
        printf("Time limit exceeded. You lose! The secret code was: %s\n", code);
    } else if (strcmp(status, "INV") == 0) {
        printf("Invalid trial format or sequence.\n");
    } else if (strcmp(status, "NOK") == 0) {
        printf("No ongoing game found for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error trying code. Please verify inputs or try again later.\n");
    } else {
        fprintf(stderr, "Unexpected server response: %s\n", status);
    }
}

void debug_game(const char *plid, unsigned int time, const char *code) {
    if (strlen(plid) != ID_SIZE || !is_number(plid)) {
        fprintf(stderr, "Invalid player ID. It must be 6 numerical digits.\n");
        return;
    }

    if (time < 0 || time > MAX_PLAYTIME) {
        fprintf(stderr, "Invalid playtime. It must be between 0 and 600 seconds.\n");
        return;
    }

    if (strlen(code) != MAX_COLORS) {
        fprintf(stderr, "Invalid code length. It must be exactly 4 characters.\n");
        return;
    }

    for (int i = 0; i < MAX_COLORS; i++) {
        if (strchr(COLOR_OPTIONS, code[i]) == NULL) {
            fprintf(stderr, "Invalid color code. Use only R, G, B, Y, O, or P.\n");
            return;
        }
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "DBG %s %03d %s\n", plid, time, code); // Time padded to 3 digits

    char response[BUFFER_SIZE];
    printf("[DEBUG] Sending debug game request: %s", message);

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        fprintf(stderr, "Error communicating with the server.\n");
        return;
    }

    printf("[DEBUG] Received response: %s", response);
    receive_debug_msg(plid, response);
}

void receive_debug_msg(const char *plid, const char *response) {
    char status[BUFFER_SIZE];
    memset(status, 0, sizeof(status));

    if (sscanf(response, "RDB %s", status) != 1) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
    }
    if (strcmp(status, "OK") == 0) {
        printf("Debug game started successfully! You can begin playing.\n");
        currPlayer = 1;
        currTries = 0;
        strcpy(plidCurr, plid);
    } else if (strcmp(status, "NOK") == 0) {
        printf("Debug game not started: an ongoing game exists for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error starting debug game. Please check your inputs or try again later.\n");
    } else {
        fprintf(stderr, "Unexpected server response: %s\n", status);
    }
}

void quit_game() {

    if (currPlayer == 0) {
        printf("Successfully quit the game.\n");
        return;
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "QUT %s\n", plidCurr);

    char response[BUFFER_SIZE];
    printf("[DEBUG] Sending quit request: %s", message); // Debug log

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        fprintf(stderr, "Error communicating with the server.\n");
        return;
    }

    printf("[DEBUG] Received response: %s", response); // Debug log
    receive_quit_msg(response);
}

void receive_quit_msg(const char *response) {
    char status[BUFFER_SIZE];
    memset(status, 0, sizeof(status));
    char secret_code[2*MAX_COLORS];
    memset(secret_code, 0, sizeof(secret_code));

    int matched = sscanf(response, "RQT %s %[^\n]", status, secret_code);
    if (matched < 1) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
    }

    if (strcmp(status, "OK") == 0 && matched == 2) {
        printf("Game successfully quit. The secret code was: %s\n", secret_code);
    } else if (strcmp(status, "NOK") == 0) {
        printf("No ongoing game found for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error quitting game. Please verify your inputs or try again later.\n");
    } else {
        fprintf(stderr, "Unexpected server response: %s\n", status);
    }

    currPlayer = 0;
    currTries = 0;
    plidCurr[0] = '\0';
}

void exit_game() {
    quit_game();
    printf("Exiting the application.\n");
}

void end_game() {
    currPlayer = 0;
    currTries = 0;
    plidCurr[0] = '\0';
    // temos que dar flush do que tiver nos buffers
    //TODO
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

void send_show_trials_msg(int fd) {
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "STR %d\n", currPlayer);

    if (send_tcp_message(fd, message) == -1) {
        fprintf(stderr, "ERROR: Failed to send 'show_trials' message\n");
    }

    receive_show_trials_msg(fd);
}

void receive_show_trials_msg(int fd) {
    char response[BUFFER_SIZE * 10];
    if (read_tcp_socket(fd, response, sizeof(response)) == -1) {
        fprintf(stderr, "ERROR: Failed to receive 'show_trials' response\n");
        return;
    }

    char status[10], filename[1024]; //TODO not sure o tamanho que ponho
    int file_size;
    char *file_data = NULL;

    if (sscanf(response, "RST %s %s %d\n", status, filename, &file_size) >= 1) {
        if (strcmp(status, "ACT") == 0 || strcmp(status, "FIN") == 0) {
            file_data = strstr(response, "\n\n") + 2;
            if (!file_data || strlen(file_data) != (size_t)file_size) {
                fprintf(stderr, "Incomplete file data.\n");
                return;
            }

            FILE *fp = fopen(filename, "w");
            if (!fp) {
                perror("Error saving file");
                return;
            }
            fwrite(file_data, 1, file_size, fp);
            fclose(fp);

            printf("Trials saved to '%s'.\n", filename);
            printf("Game Summary:\n%s\n", file_data);
        } else if (strcmp(status, "NOK") == 0) {
            printf("No game data available for player '%d'.\n", currPlayer);
        } else {
            printf("Unexpected server response: %s\n", response);
        }
    } else {
        printf("Invalid response format.\n");
    }
}

void send_show_scoreboard_msg(int fd) {
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "SSB\n");

    if (send_tcp_message(fd, message) == -1) {
        fprintf(stderr, "ERROR: Failed to send 'show_scoreboard' message\n");
    }

    receive_show_scoreboard_msg(fd);
}

void receive_show_scoreboard_msg(int fd) {
    char response[BUFFER_SIZE * 10];
    if (read_tcp_socket(fd, response, sizeof(response)) == -1) {
        fprintf(stderr, "ERROR: Failed to receive 'show_scoreboard' response\n");
        return;
    }

    char status[10], filename[1024]; //TODO
    int file_size;
    char *file_data = NULL;

    if (sscanf(response, "RSS %s %s %d\n", status, filename, &file_size) >= 1) {
        if (strcmp(status, "OK") == 0) {
            file_data = strstr(response, "\n\n") + 2;
            if (!file_data || strlen(file_data) != (size_t)file_size) {
                fprintf(stderr, "Incomplete file data.\n");
                return;
            }

            FILE *fp = fopen(filename, "w");
            if (!fp) {
                perror("Error saving file");
                return;
            }
            fwrite(file_data, 1, file_size, fp);
            fclose(fp);

            printf("Scoreboard saved to '%s'.\n", filename);
            printf("Top 10 Scores:\n%s\n", file_data);
        } else if (strcmp(status, "EMPTY") == 0) {
            printf("The scoreboard is empty.\n");
        } else {
            printf("Unexpected server response: %s\n", response);
        }
    } else {
        printf("Invalid response format.\n");
    }
}
