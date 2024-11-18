#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "player.h"
#include "common.h"


//Global variables, they will never change.
char *GSIP = DEFAULT_IP;                 // Default IP
int GSport = DEFAULT_PORT;               // Default port
char plidCurr[ID_SIZE + 1];              // Current player ID
int currPlayer = 0;                      // Flag to check if a player is playing or not
int currTries = 1;                       // Number of tries of the current player (it starts with the "1st try")
//TODO eu presumo que apos um startgame este terminal só pode usar outro plid se fizer quit

void main_arguments(int argc, char *argv[]){
    int opt, ip_set = 0, port_set = 0;
    while ((opt = getopt(argc, argv, "n:p:")) != -1) {
        switch (opt) {
            case 'n':
                if (ip_set) {
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                if (is_valid_ip(optarg)) {
                    GSIP = optarg;
                } else {
                    fprintf(stderr, "Error: Invalid IP address\n");
                    exit(EXIT_FAILURE);
                }
                ip_set = 1;
                break;

            case 'p':
                if (port_set) {
                    usage(argv[0]);
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
                exit(EXIT_FAILURE);
        }
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

int main(int argc, char *argv[]) {

    main_arguments(argc, argv);

    while (1) {
        printf("Enter command: \n");
        char input[MAX_COMMAND_SIZE];
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            fprintf(stderr, "Error reading command.\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        Command cmd = get_next_command(input);
        
        switch (cmd) {
            case CMD_START: {
                char plid[MAX_COMMAND_SIZE];
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
                char code[MAX_COMMAND_SIZE];
                if (sscanf(input, "try %s", code) != 1) {
                    fprintf(stderr, "Invalid 'try' command format. Format: try C1 C2 C3 C4\n");
                    break;
                }
                try_code(code);
                break;
            }

            /* TCP  TODO
            case CMD_SHOW_TRIALS:
                show_trials();
                break;

            case CMD_SCOREBOARD:
                show_scoreboard();
                break;
            */

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

                char plid[MAX_COMMAND_SIZE];
                unsigned int max_playtime;
                char code[MAX_COMMAND_SIZE];

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
    snprintf(message, sizeof(message), "SNG %s %03d\n", plid, time);

    char response[BUFFER_SIZE];
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
    char trimmed_code[MAX_COLORS + 1];
    snprintf(trimmed_code, sizeof(trimmed_code), "%.*s", MAX_COLORS, code); // Ensure exact length

    if (strlen(trimmed_code) != MAX_COLORS) {
        fprintf(stderr, "Invalid code length. It must be exactly 4 characters.\n");
        return;
    }

    for (int i = 0; i < MAX_COLORS; i++) {
        if (strchr(COLOR_OPTIONS, trimmed_code[i]) == NULL) {
            fprintf(stderr, "Invalid color code. Use only R, G, B, Y, O, or P.\n");
            return;
        }
    }

    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "TRY %s %s %d\n",plidCurr, trimmed_code, currTries);

    char response[BUFFER_SIZE];
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
    char code[MAX_COLORS + 1] = {0};

    if (sscanf(response, "RTR %s %d %d %4s", status, &black, &white, code) < 3) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
    }

    if (strcmp(status, "OK") == 0) {
        printf("Tries: %d, Black: %d, White: %d\n", currTries, black, white);
        currTries++;

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
    snprintf(message, sizeof(message), "DBG %s %03d %s\n", plid, time, code); // Time padded to 3 digits

    char response[BUFFER_SIZE];
    printf("[DEBUG] Sending debug game request: %s", message);

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        fprintf(stderr, "Error communicating with the server.\n");
        return;
    }

    printf("[DEBUG] Received response: %s", response);
    receive_debug_msg(response);
}

void receive_debug_msg(const char *response) {
    char status[BUFFER_SIZE];

    if (sscanf(response, "RDB %s", status) != 1) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
    }
    if (strcmp(status, "OK") == 0) {
        printf("Debug game started successfully! You can begin playing.\n");
    } else if (strcmp(status, "NOK") == 0) {
        printf("Debug game not started: an ongoing game exists for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error starting debug game. Please check your inputs or try again later.\n");
    } else {
        fprintf(stderr, "Unexpected server response: %s\n", status);
    }
}

void quit_game() {

    char message[BUFFER_SIZE];
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
    char secret_code[MAX_COLORS + 1] = {0};

    int matched = sscanf(response, "RQT %s %4s", status, secret_code);
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

// Function to show trials
void show_trials() {
    // TCP connection to GS and request trials list
    printf("Showing trials...\n");
    // TCP request and response handling here (implementation-specific)
}

// Function to show scoreboard
void show_scoreboard() {
    // TCP connection to GS and request scoreboard
    printf("Showing scoreboard...\n");
    // TCP request and response handling here (implementation-specific)
}
