#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

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
    time_t current_time;

    if (time(&current_time) - player->current_game->start_time > player->current_game->max_time) {
        char temp[2*MAX_COLORS];
        convert_code(temp, player->current_game->secret_key, SECRET_TO_CODE);
        printf("[DEBUG] temp: %s\n", temp);
        snprintf(response, sizeof(response), "RTR ETM -1 -1 -1 %s\n", temp); //TODO corrigir isto, é temporário o fix
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        end_game(player);
        return;
    }

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
        char temp[2*MAX_COLORS];
        convert_code(temp, player->current_game->secret_key, SECRET_TO_CODE);
        printf("[DEBUG] temp: %s\n", temp);
        snprintf(response, sizeof(response), "RTR ENT -1 -1 -1 %s\n", temp); //TODO corrigir isto, é temporário o fix
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        end_game(player);
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

    FILE *fp = fopen(player->current_game->filename, "a");
    if (!fp) {
        perror("Error opening file");
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%d: %s %d %d %ld\n",
            trial_num, guess, black, white, current_time);
    fwrite(buffer, 1, strlen(buffer), fp);
    fclose(fp);

    send_udp_response(response, client_addr, client_addr_len, udp_socket);
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