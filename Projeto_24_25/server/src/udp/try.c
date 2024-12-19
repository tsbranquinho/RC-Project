#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_try_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 100];
    char aux_guess[2*MAX_COLORS]; // will be stored in check_try_err
    int trial_num;
    memset(plid, 0, sizeof(plid));
    memset(aux_guess, 0, sizeof(aux_guess));
    aux_guess[1] = ' ';
    aux_guess[3] = ' ';
    aux_guess[5] = ' ';
    int n_args = sscanf(request, "TRY %s %c %c %c %c %d", plid, &aux_guess[0], &aux_guess[2], &aux_guess[4], &aux_guess[6], &trial_num);
    aux_guess[7] = '\0';


    if (check_try_err(request, n_args, aux_guess, &trial_num) < 0) {
        if(settings.verbose_mode) {
            printf("Message: %s\n", request);
            fprintf(stderr, "Invalid try request\n");
        }
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    if(settings.verbose_mode) {
        printf("PLID: %s\n", plid);
        printf("Trial number: %d\n", trial_num);
        printf("Guess: %s\n", aux_guess);
    }

    char guess[MAX_COLORS + 1];
    for (int i = 0; i < MAX_COLORS; i++) {
        guess[i] = aux_guess[2*i];
    }
    guess[MAX_COLORS] = '\0';

    // NOK
    Player *player = find_player(plid);
    if (check_try_nok(plid, player) < 0) {
        send_udp_response("RTR NOK\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    player->current_game->trial_count++;
    int black = 0, white = 0;
    char response[SMALL_BUFFER];

    // ETM
    if (check_try_etm(player, response) < 0) {
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        end_game(player, plid_mutex);
        return;
    }

    //INV
    int ret = check_try_inv(player, trial_num, guess);
    if (ret == -1) {
        send_udp_response("RTR INV\n", client_addr, client_addr_len, udp_socket);
        player->current_game->trial_count--;
        mutex_unlock(plid_mutex);
        return;
    }
    else if (ret == 1) {
        snprintf(response, sizeof(response), "RTR OK %d %d %d\n", trial_num, player->current_game->trial->black, player->current_game->trial->white);
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        player->current_game->trial_count--;
        mutex_unlock(plid_mutex);
        return;
    }

    //DUP
    if (check_try_dup(player, guess) < 0) {
        send_udp_response("RTR DUP\n", client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        return;
    }

    //ENT
    if (check_try_ent(player, response, plid_mutex) < 0) {
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        return;
    }

    if (calculate_feedback(guess, player->current_game->secret_key, &black, &white) < 0) {
        player->current_game->trial_count--;
        send_udp_response("RTR ERR\n \0", client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        return;
    }
    
    if (write_try_to_file(player, guess, black, white) < 0) {
        player->current_game->trial_count--;
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        return;
    }

    if (black == MAX_COLORS) {
        //TODO: fechar o jogo e limpar do lado do servidor
        snprintf(response, sizeof(response), "RTR OK %d 4 0\n", trial_num);
        player->current_game->end_status = 'W';
        score_game(player);
        end_game(player, plid_mutex);
    } else {
        if (player->current_game->trial_count == MAX_TRIALS) {
            player->current_game->trial_count++; //Está errado e excedemos o limite de tentativas
            if (check_try_ent(player, response, plid_mutex) < 0) {
                send_udp_response(response, client_addr, client_addr_len, udp_socket);
                mutex_unlock(plid_mutex);
                return;
            }
        }
        snprintf(response, sizeof(response), "RTR OK %d %d %d\n", trial_num, black, white);
    }

    send_udp_response(response, client_addr, client_addr_len, udp_socket);
    mutex_unlock(plid_mutex);
}

int check_try_err(const char *request, int n_args, char *aux_guess, int *trial_num) {
    if (n_args != 6) {
        return -1;
    }

    aux_guess[2*MAX_COLORS-1] = '\0';

    if (strlen(aux_guess) != 2*MAX_COLORS-1) {
        return -1;
    }
    for (int i = 0; i < 2*MAX_COLORS; i += 2) {
        if (strchr(COLOR_OPTIONS, aux_guess[i]) == NULL) {
            return -1;
        }
        if (aux_guess[i+1] != ' ' && aux_guess[i+1] != '\0') {
            return -1;
        }
    }
    return 0;
}

int check_try_nok(const char *plid, Player *player) {
    if (!player) {
        return -1;
    }
    return 0;
}

int check_try_etm(Player *player, char* response) {
    if (time(&player->current_game->last_time) - player->current_game->start_time > player->current_game->max_time) {
        char temp[2*MAX_COLORS];
        convert_code(temp, player->current_game->secret_key, SECRET_TO_CODE);
        snprintf(response, SMALL_BUFFER, "RTR ETM %s\n", temp); //TODO corrigir isto, é temporário o fix
        player->current_game->end_status = 'T';
        return -1;
    }
    return 0;
}

int check_try_inv(Player *player, int trial_num, char *guess) {
    if (player->current_game->trial_count == trial_num + 1) {
        // pode ser Invalid, o jogo estar um à frente do suposto
        Trials *aux = malloc(sizeof(Trials));
        strcpy(aux->guess, guess);
        if (player->current_game->trial != NULL) {
            if (strcmp(player->current_game->trial->guess, guess) != 0) {
                return -1;
            }
            else {
                player->current_game->trial_count--;
                return 1;
            }
        }
        else {
            free(aux);
            return -1;
        }
    }
    return 0;
}

int check_try_dup(Player *player, char *guess) {
    for (Trials *trial = player->current_game->trial; trial != NULL; trial = trial->prev) {
        if (strcmp(trial->guess, guess) == 0) {
            player->current_game->trial_count--;
            return -1;
        }
    }
    return 0;
}

int check_try_ent(Player *player, char* response, pthread_mutex_t *plid_mutex) {
    if (player->current_game->trial_count > MAX_TRIALS) {
        char temp[2*MAX_COLORS];
        convert_code(temp, player->current_game->secret_key, SECRET_TO_CODE);
        snprintf(response, SMALL_BUFFER, "RTR ENT %s\n", temp); //TODO corrigir isto, é temporário o fix
        player->current_game->end_status = 'F';
        end_game(player, plid_mutex);
        return -1;
    }
    return 0;
}

int calculate_feedback(const char *guess, const char *secret, int *black, int *white) {

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

void create_trial(Player *player, char *guess, int black, int white) {
    Trials *trial = malloc(sizeof(Trials)); //TODO: check if malloc fails
    memset(trial->guess, 0, sizeof(trial->guess));
    strncpy(trial->guess, guess, MAX_COLORS);
    trial->black = black;
    trial->white = white;
    trial->prev = player->current_game->trial;
    player->current_game->trial = trial;
}

int write_try_to_file(Player *player, char *guess, int black, int white) {
    create_trial(player, guess, black, white);
    FILE *fp = fopen(player->current_game->filename, "a");
    if (!fp) {
        perror("Error opening file");
        return -1;
    }
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "T: %s %d %d %ld\n",
            guess, black, white, player->current_game->last_time - player->current_game->start_time);
    if (fwrite(buffer, 1, strlen(buffer), fp) != strlen(buffer)) {
        perror("Error writing to file");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;

}