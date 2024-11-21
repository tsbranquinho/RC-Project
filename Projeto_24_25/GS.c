#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>  // For socket functions
#include <sys/socket.h>  // For socket functions
#include <sys/types.h>  // For data types
#include <netinet/in.h>  // For sockaddr_in
#include <time.h>  // For time functions
#include "GS.h"
#include "common.h"


Player *hash_table[MAX_PLAYERS] = {NULL};

void usage(const char *progname) {
    fprintf(stderr, "Usage: %s [-p GSport] [-v]\n", progname);
    fprintf(stderr, "  -p GSport  Specify the port number (only one) of the game server (optional).\n");
    fprintf(stderr, "  -v         Enable verbose mode (optional).\n");
}

int main(int argc, char *argv[]) {
    int GSport = DEFAULT_PORT;
    int verbose_mode = 0;
    int port_set = 0;
    int opt;

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

void handle_start_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];
    int max_time;
    printf("Got a start request\n");

    if (sscanf(request, "SNG %6s %3d", plid, &max_time) != 2 || max_time <= 0 || max_time > MAX_PLAYTIME) {
        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }
    
    Player *player = find_player(plid);
    if (player != NULL && player->is_playing) {
        send_udp_response("RSG NOK\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    if (player == NULL) {
        player = create_player(plid);
        insert_player(player);
    }

    player->is_playing = 1;
    player->current_game = malloc(sizeof(Game));
    player->current_game->start_time = time(NULL);
    player->current_game->trial_count = 0;
    player->current_game->max_time = max_time;
    generate_random_key(player->current_game->secret_key);
    player->current_game->trial = NULL;


    send_udp_response("RSG OK\n", client_addr, client_addr_len, udp_socket);
}

/* TODO FALTA FAZER ESTAS DUP/INV/ENT/ETM
DUP, if the secret key guess repeats a previous trial’s guess. In this case
the number of trials is not increased;
• INV, if the trial number nT is the expected value minus 1, but the secret
key guess is different from the one of the previous message, or if the trial
number nT is not the expected value;
• ENT, if no more attempts available. In this case the secret key is revealed,
sending the corresponding colour code: C1 C2 C3 C4;
  11/11/2024
• ETM, if the maximum play time has been exceeded. In this case the secret
key is revealed, sending the corresponding colour code: C1 C2 C3 C4;
*/

void handle_try_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];
    char aux_guess[2*MAX_COLORS];
    int trial_num;

    if (sscanf(request, "TRY %6s %[^0-9] %d", plid, aux_guess, &trial_num) != 3) {
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    aux_guess[2*MAX_COLORS-1] = '\0';

    if (strlen(aux_guess) != 2*MAX_COLORS-1) {
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }
    for (int i = 0; i < 2*MAX_COLORS; i += 2) {
        if (strchr(COLOR_OPTIONS, aux_guess[i]) == NULL) {
            send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
            return;
        }
        if (aux_guess[i+1] != ' ' && aux_guess[i+1] != '\0') {
            send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
            return;
        }
    }

    char guess[MAX_COLORS + 1];
    for (int i = 0; i < MAX_COLORS; i++) {
        guess[i] = aux_guess[2*i];
    }
    guess[MAX_COLORS] = '\0';

    Player *player = find_player(plid);
    player->current_game->trial_count++;
    if (!player || !player->is_playing) {
        send_udp_response("RTR NOK\n", client_addr, client_addr_len, udp_socket);
        return;
    }


    int black = 0, white = 0;
    char response[BUFFER_SIZE];

    if (player->current_game->trial_count == trial_num + 1) {
        // pode ser Invalid, o jogo estar um à frente do suposto
        Trials *aux = malloc(sizeof(Trials));
        strcpy(aux->guess, guess);
        if (player->current_game->trial != NULL) {
            if (strcmp(player->current_game->trial->guess, guess) != 0) {
                send_udp_response("RTR INV\n", client_addr, client_addr_len, udp_socket);
                player->current_game->trial_count--;
            }
            else {
                player->current_game->trial_count--;
                snprintf(response, sizeof(response), "RTR OK %d %d %d\n", trial_num, black, white);
                send_udp_response(response, client_addr, client_addr_len, udp_socket);
            }
        }
        else {
            send_udp_response("RTR INV\n", client_addr, client_addr_len, udp_socket);
            player->current_game->trial_count--;
        }
        free(aux);
        return;
    }

    for (Trials *trial = player->current_game->trial; trial != NULL; trial = trial->prev) {
        if (strcmp(trial->guess, guess) == 0) {
            player->current_game->trial_count--;
            send_udp_response("RTR DUP\n", client_addr, client_addr_len, udp_socket);
            return;
        }
    }

    if (player->current_game->trial_count > MAX_TRIALS) {
        char temp[MAX_COLORS + 1];
        convert_code(temp, player->current_game->secret_key);
        printf("[DEBUG] temp: %s\n", temp);
        snprintf(response, sizeof(response), "RTR ENT -1 -1 -1 %s\n", temp); //TODO corrigir isto, é temporário o fix
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        player->current_game->trial_count--;
        return;
    }

    if (time(NULL) - player->current_game->start_time > player->current_game->max_time) {
        char temp[MAX_COLORS + 1];
        convert_code(temp, player->current_game->secret_key);
        printf("[DEBUG] temp: %s\n", temp);
        snprintf(response, sizeof(response), "RTR ETM -1 -1 -1 %s\n", temp); //TODO corrigir isto, é temporário o fix
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        player->current_game->trial_count--;
        return;
    }

    if (calculate_feedback(guess, player->current_game->secret_key, &black, &white) < 0) {
        send_udp_response("RTR ERR\n \0", client_addr, client_addr_len, udp_socket);
        return;
    }

    if (black == MAX_COLORS) {
        snprintf(response, sizeof(response), "RTR OK %d 4 0\n", trial_num);
        player->is_playing = 0;
    } else {
        snprintf(response, sizeof(response), "RTR OK %d %d %d\n", trial_num, black, white);
    }
    Trials *trial = malloc(sizeof(Trials));
    strncpy(trial->guess, guess, MAX_COLORS);
    trial->black = black;
    trial->white = white;
    trial->prev = player->current_game->trial;
    player->current_game->trial = trial;

    send_udp_response(response, client_addr, client_addr_len, udp_socket);
}

void handle_quit_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];

    if (sscanf(request, "QUT %6s", plid) != 1) {
        send_udp_response("RQT ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    Player *player = find_player(plid);
    if (!player || !player->is_playing) {
        send_udp_response("RQT NOK\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "RQT OK %s\n", player->current_game->secret_key);
    for (Trials *trial = player->current_game->trial; trial != NULL; trial = trial->prev) {
        free(trial);
    }
    free(player->current_game);
    player->is_playing = 0;
    player->current_game = NULL;

    send_udp_response(response, client_addr, client_addr_len, udp_socket);
}

void handle_debug_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];
    int max_time;
    char key[MAX_COLORS + 1];

    if (sscanf(request, "DBG %6s %3d %4s", plid, &max_time, key) != 3 || max_time <= 0 || max_time > MAX_PLAYTIME) {
        send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    Player *player = find_player(plid);
    if (player != NULL && player->is_playing) {
        send_udp_response("RDB NOK\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    if (player == NULL) {
        player = create_player(plid);
        insert_player(player);
    }

    player->is_playing = 1;
    player->current_game = malloc(sizeof(Game));
    player->current_game->start_time = time(NULL);
    player->current_game->trial_count = 0;
    player->current_game->max_time = max_time;
    player->current_game->trial = NULL;
    strncpy(player->current_game->secret_key, key, MAX_COLORS);

    send_udp_response("RDB OK\n", client_addr, client_addr_len, udp_socket);
}

void send_udp_response(const char *message, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    if (sendto(udp_socket, message, strlen(message), 0, (struct sockaddr *)client_addr, client_addr_len) < 0) {
        perror("Failed to send UDP response");
    }
}

void generate_random_key(char *key) {
    const char *colors = COLOR_OPTIONS;
    srand(time(NULL));
    for (int i = 0; i < MAX_COLORS; i++) {
        key[i] = colors[rand() % strlen(colors)];
    }
    key[MAX_COLORS] = '\0';
}

int color_to_index(char color) {
    switch(color) {
        case 'R': return 0;
        case 'G': return 1;
        case 'B': return 2;
        case 'Y': return 3;
        case 'O': return 4;
        case 'P': return 5;
        default: return -1; // Invalid color
    }
}

int calculate_feedback(const char *guess, const char *secret, int *black, int *white) {
    printf("Guess: %s\n", guess);
    printf("Secret: %s\n", secret);

    int guess_count[MAX_COLORS] = {0};  // Array for counting occurrences of each color in the guess
    int secret_count[MAX_COLORS] = {0}; // Array for counting occurrences of each color in the secret

    *black = 0;
    *white = 0;

    // Count "black" pegs and populate color counts for non-matching positions
    for (int i = 0; i < MAX_COLORS; i++) {
        if (strchr(COLOR_OPTIONS, guess[i]) == NULL || strchr(COLOR_OPTIONS, secret[i]) == NULL) {
            fprintf(stderr, "Error: Invalid character in guess or secret.\n");
            return -1;
        }

        if (guess[i] == secret[i]) {
            (*black)++;
        } else {
            guess_count[color_to_index(guess[i])]++;
            secret_count[color_to_index(secret[i])]++;
        }
    }

    // Count "white" pegs (min of guess and secret counts for each color)
    for (int i = 0; i < MAX_COLORS; i++) {
        *white += (guess_count[i] < secret_count[i]) ? guess_count[i] : secret_count[i];
    }

    return 0; // Success
}

Player *find_player(const char *plid) {
    unsigned int index = hash(plid);
    Player *current = hash_table[index];

    while (current != NULL) {
        if (strcmp(current->plid, plid) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void remove_player(const char *plid) {
    unsigned int index = hash(plid);
    Player *current = hash_table[index];
    Player *prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->plid, plid) == 0) {
            if (prev == NULL) {
                hash_table[index] = current->next;  // Remove from the head of the list
            } else {
                prev->next = current->next;  // Remove from the middle or end of the list
            }
            free(current->current_game);  // Free the associated game
            free(current);  // Free the player
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("Player with ID %s not found\n", plid);  // Player not found
}

Player *create_player(const char *plid) {
    Player *player = malloc(sizeof(Player));
    strncpy(player->plid, plid, ID_SIZE);
    player->plid[ID_SIZE] = '\0';
    player->is_playing = 0;
    player->current_game = NULL;
    player->next = NULL;  // No next player yet
    return player;
}

unsigned int hash(const char *plid) {
    unsigned int hash_value = 0;
    while (*plid) {
        hash_value = (hash_value << 5) + *plid++;
    }
    return hash_value % MAX_PLAYERS;
}

void insert_player(Player *player) {
    unsigned int index = hash(player->plid);
    player->next = hash_table[index];
    hash_table[index] = player;
}

void convert_code(char *temp, char *secret) {
    for (int i = 0; i < MAX_COLORS; i++) {
        temp[2*i] = secret[i];
        temp[2*i+1] = ' ';
    }
    temp[2*MAX_COLORS-1] = '\0';
}