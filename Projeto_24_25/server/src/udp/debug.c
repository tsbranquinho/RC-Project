#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_debug_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];
    int max_time;
    char aux[MAX_COLORS+1];
    memset(plid, 0, sizeof(plid));
    memset(aux, 0, sizeof(aux));

    if (sscanf(request, "DBG %6s %3d %c %c %c %c", plid, &max_time, &aux[0], &aux[1], &aux[2], &aux[3]) != 6 || max_time <= 0 || max_time > MAX_PLAYTIME) {
        send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    if(!valid_plid(plid)) {
        send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    Player *player = find_player(plid);
    if (player != NULL && player->current_game->trial != NULL) {
        
        pthread_mutex_t *plid_mutex = mutex_plid(plid);
        if (!plid_mutex) {
            send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
            return ERROR;
        }

        send_udp_response("RDB NOK\n", client_addr, client_addr_len, udp_socket);
        mutex_unlock(plid_mutex);
        set_verbose_debug_message(request, plid, max_time, aux);
        return SUCCESS;
    }

    
    if (player == NULL) {
        player = create_player(plid);
        if (!player) {
            fprintf(stderr, "Failed to create player %s\n", plid);
            send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
            return ERROR;
        }
        insert_player(player);
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    player->current_game = malloc(sizeof(Game));
    if (!player->current_game) {
        send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }
    player->current_game->start_time = time(NULL);
    player->current_game->trial_count = 0;
    player->current_game->max_time = max_time;
    player->current_game->trial = NULL;
    player->current_game->mode = DEBUG;
    strncpy(player->current_game->secret_key, aux, MAX_COLORS+1);

    //GAME_XXXXXX.txt
    char filename[GAME_FILE_NAME_SIZE+6]; // 6 for "GAMES/"
    snprintf(filename, sizeof(filename), "GAMES/GAME_%s.txt", plid);
    strncpy(player->current_game->filename, filename, sizeof(player->current_game->filename));
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Error creating file");

        mutex_unlock(plid_mutex);

        send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    char time_str[20];
    struct tm *current_time = gmtime(&player->current_game->start_time);

    sprintf(time_str, "%4d-%02d-%02d %02d:%02d:%02d", current_time->tm_year + 1900,
                current_time->tm_mon + 1, current_time->tm_mday, current_time->tm_hour,
                        current_time->tm_min, current_time->tm_sec);
    
    char buffer[SMALL_BUFFER];

    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s D %s %03d %s %ld\n",
            plid, player->current_game->secret_key,
            player->current_game->max_time, time_str,
            player->current_game->start_time);
    fwrite(buffer, 1, strlen(buffer), fp);
    fclose(fp);

    mutex_unlock(plid_mutex);
    send_udp_response("RDB OK\n", client_addr, client_addr_len, udp_socket);
    set_verbose_debug_message(request, plid, max_time, aux);
    return SUCCESS;
}

void set_verbose_debug_message(char *request, const char *plid, int max_time, const char *key) {
    memset(request, 0, strlen(request));
    sprintf(request, "Debug request: PLID = %s, max_time = %d, key = %c %c %c %c\n", plid, max_time, key[0], key[1], key[2], key[3]);
}