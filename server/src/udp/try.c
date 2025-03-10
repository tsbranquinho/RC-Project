#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_try_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 100];
    char aux_guess[2*MAX_COLORS]; // will be stored in check_try_err
    int trial_num;
    memset(plid, 0, sizeof(plid));
    memset(aux_guess, 0, sizeof(aux_guess));
    aux_guess[1] = ' ';
    aux_guess[3] = ' ';
    aux_guess[5] = ' ';
    aux_guess[7] = '\0';
    int n_args = sscanf(request, "TRY %s %c %c %c %c %d", plid, &aux_guess[0], &aux_guess[2], &aux_guess[4], &aux_guess[6], &trial_num);


    if (check_try_err(request, n_args, aux_guess, trial_num) < 0) {
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    if(valid_plid(plid) == ERROR) {
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
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
        set_verbose_try_message(request, plid, trial_num, aux_guess);
        return SUCCESS;
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    player->current_game->trial_count++;
    int black = 0, white = 0;
    char response[SMALL_BUFFER];

    // ETM
    if (check_try_etm(player, response) < 0) {
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        end_game(player, plid_mutex);
        set_verbose_try_message(request, plid, trial_num, aux_guess);
        return SUCCESS;
    }

    //INV
    int ret = check_try_inv(player, trial_num, guess);
    if (ret == -1) {
        send_udp_response("RTR INV\n", client_addr, client_addr_len, udp_socket);
        player->current_game->trial_count--;
        mutex_unlock(plid_mutex);
        set_verbose_try_message(request, plid, trial_num, aux_guess);
        return SUCCESS;
    }
    else if (ret == 1) {
        snprintf(response, sizeof(response), "RTR OK %d %d %d\n", trial_num, player->current_game->trial->black, player->current_game->trial->white);
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        player->current_game->trial_count--;
        mutex_unlock(plid_mutex);
        set_verbose_try_message(request, plid, trial_num, aux_guess);
        return SUCCESS;
    }

    //DUP
    if (check_try_dup(player, guess) < 0) {
        send_udp_response("RTR DUP\n", client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        set_verbose_try_message(request, plid, trial_num, aux_guess);
        return SUCCESS;
    }

    //ENT
    if (check_try_ent(player, response, plid_mutex) < 0) {
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        set_verbose_try_message(request, plid, trial_num, aux_guess);
        return SUCCESS;
    }

    if (calculate_feedback(guess, player->current_game->secret_key, &black, &white) < 0) {
        player->current_game->trial_count--;
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        return ERROR;
    }
    
    if (write_try_to_file(player, guess, black, white) < 0) {
        player->current_game->trial_count--;
        send_udp_response("RTR ERR\n", client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        return ERROR;
    }

    if (black == MAX_COLORS) {
        snprintf(response, sizeof(response), "RTR OK %d 4 0\n", trial_num);
        player->current_game->end_status = 'W';
        score_game(player);
        end_game(player, plid_mutex);
        send_udp_response(response, client_addr, client_addr_len, udp_socket);
        set_verbose_try_message(request, plid, trial_num, aux_guess);
        return SUCCESS;
    } else {
        if (player->current_game->trial_count == MAX_TRIALS) {
            player->current_game->trial_count++; //Está errado e excedemos o limite de tentativas
            if (check_try_ent(player, response, plid_mutex) < 0) {
                send_udp_response(response, client_addr, client_addr_len, udp_socket);
                mutex_unlock(plid_mutex);
                set_verbose_try_message(request, plid, trial_num, aux_guess);
                return SUCCESS;
            }
        }
        snprintf(response, sizeof(response), "RTR OK %d %d %d\n", trial_num, black, white);
    }

    send_udp_response(response, client_addr, client_addr_len, udp_socket);
    mutex_unlock(plid_mutex);
    set_verbose_try_message(request, plid, trial_num, aux_guess);
    return SUCCESS;
}

int check_try_err(const char *request, int n_args, char *aux_guess, int trial_num) {
    if (n_args != 6 || strlen(request) > TRY_MSG_SIZE) {
        return ERROR;
    }

    if (trial_num < 0 || trial_num > MAX_TRIALS) {
        return ERROR;
    }

    if (strlen(aux_guess) != 2*MAX_COLORS-1) {
        return ERROR;
    }
    for (int i = 0; i < 2*MAX_COLORS; i += 2) {
        if (strchr(COLOR_OPTIONS, aux_guess[i]) == NULL) {
            return ERROR;
        }
        if (aux_guess[i+1] != ' ' && aux_guess[i+1] != '\0') {
            return ERROR;
        }
    }
    return 0;
}

int check_try_nok(const char *plid, Player *player) {
    if (!player) {
        return ERROR;
    }
    return 0;
}

int check_try_etm(Player *player, char* response) {
    if (time(&player->current_game->last_time) - player->current_game->start_time > player->current_game->max_time) {
        char temp[2*MAX_COLORS];
        convert_code(temp, player->current_game->secret_key, SECRET_TO_CODE);
        snprintf(response, SMALL_BUFFER, "RTR ETM %s\n", temp);
        player->current_game->end_status = 'T';
        return ERROR;
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
                return ERROR;
            }
            else {
                return 1;
            }
        }
        else {
            free(aux);
            return ERROR;
        }
    }
    return 0;
}

int check_try_dup(Player *player, char *guess) {
    for (Trials *trial = player->current_game->trial; trial != NULL; trial = trial->prev) {
        if (strcmp(trial->guess, guess) == 0) {
            player->current_game->trial_count--;
            return ERROR;
        }
    }
    return 0;
}

int check_try_ent(Player *player, char* response, pthread_mutex_t *plid_mutex) {
    if (player->current_game->trial_count > MAX_TRIALS) {
        char temp[2*MAX_COLORS];
        convert_code(temp, player->current_game->secret_key, SECRET_TO_CODE);
        snprintf(response, SMALL_BUFFER, "RTR ENT %s\n", temp);
        player->current_game->end_status = 'F';
        end_game(player, plid_mutex);
        return ERROR;
    }
    return 0;
}

int calculate_feedback(const char *guess, const char *secret, int *black, int *white) {

    char copy_secret[MAX_COLORS + 1];
    int calculated[MAX_COLORS] = {0};
    memset(copy_secret, 0, sizeof(copy_secret));
    strcpy(copy_secret, secret);
    for (int i = 0; i < MAX_COLORS; i++) {
        if (guess[i] == secret[i]) {
            (*black)++;
            copy_secret[i] = ' ';
            calculated[i] = 1;
            continue;
        }
    }
    for (int i = 0; i < MAX_COLORS; i++) {
        if (calculated[i]) {
            continue;
        }
        for (int j = 0; j < MAX_COLORS; j++) {
            if (guess[i] == copy_secret[j]) {
                (*white)++;
                copy_secret[j] = ' ';
                break;
            }
        }
    }
    return 0;
}

void create_trial(Player *player, char *guess, int black, int white) {

    Trials *trial = malloc(sizeof(Trials));

    if(!trial) {
        if (settings.verbose_mode) {
            perror("Failed to create trial");
        }
        return;
    }

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
        if (settings.verbose_mode) {
            perror("Error opening file");
        }
        return ERROR;
    }
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "T: %s %d %d %ld\n",
            guess, black, white, player->current_game->last_time - player->current_game->start_time);
    if (fwrite(buffer, 1, strlen(buffer), fp) != strlen(buffer)) {
        if (settings.verbose_mode) {
            perror("Error writing to file");
        }
        fclose(fp);
        return ERROR;
    }
    fclose(fp);
    return 0;
}

void set_verbose_try_message(char* request, const char* plid, int trial_num, const char* guess) {
    memset(request, 0, strlen(request));
    sprintf(request, "Try request: PLID = %s, trial_num = %d, guess = %s\n", plid, trial_num, guess);
}