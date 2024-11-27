#include "../include/constants.h"
#include "../include/prototypes.h"
#include "../include/globals.h"

char*GSIP = DEFAULT_IP;                 // Default IP
int GSport = DEFAULT_PORT;               // Default port
int currPlayer = 0;                      // Flag to check if a player is playing or not
int currTries = 0;                       // Number of tries of the current player (it starts with the "1st try")
char plidCurr[ID_SIZE + 1];              // Current player ID

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