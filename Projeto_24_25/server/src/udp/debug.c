#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_debug_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];
    int max_time;
    char key[2*MAX_COLORS];
    char aux[MAX_COLORS+1];
    memset(plid, 0, sizeof(plid));
    memset(key, 0, sizeof(key));

    //TODO verificar se isto está bem
    printf("DEBUG REQUEST: %s\n", request);

    if (sscanf(request, "DBG %6s %3d %[^\n]", plid, &max_time, key) != 3 || max_time <= 0 || max_time > MAX_PLAYTIME) {
        send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    for (int i = 0; i < MAX_COLORS; i++) {
        if (key[2*i+1] != ' ' && key[2*i+1] != '\0' && strchr(COLOR_OPTIONS, key[2*i]) != NULL) {
            send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
            return;
        }
        aux[i] = key[2*i];
    }

    Player *player = find_player(plid);
    if (player != NULL && player->current_game->trial != NULL) {
        
        pthread_mutex_t *plid_mutex = mutex_plid(plid);
        if (!plid_mutex) {
            send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
            return;
        }

        send_udp_response("RSG NOK\n", client_addr, client_addr_len, udp_socket);
        
        mutex_unlock(plid_mutex);
        return;
    }

    
    if (player == NULL) {
        player = create_player(plid);
        if (!player) {
            fprintf(stderr, "Failed to create player %s\n", plid);
            send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
            return;
        }
        insert_player(player);
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    player->current_game = malloc(sizeof(Game));
    player->current_game->start_time = time(NULL);
    player->current_game->trial_count = 0;
    player->current_game->max_time = max_time;
    player->current_game->trial = NULL;
    player->current_game->mode = DEBUG;
    strncpy(player->current_game->secret_key, aux, MAX_COLORS+1);

    //TODO create file
    //GAME_XXXXXX.txt
    char filename[GAME_FILE_NAME_SIZE+6]; // 6 for "GAMES/"
    snprintf(filename, sizeof(filename), "GAMES/GAME_%s.txt", plid);
    strncpy(player->current_game->filename, filename, sizeof(player->current_game->filename));
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Error creating file");

        mutex_unlock(plid_mutex);

        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    char time_str[20];
    struct tm *current_time = gmtime(&player->current_game->start_time);

    sprintf(time_str, "%4d-%02d-%02d %02d:%02d:%02d", current_time->tm_year + 1900,
                current_time->tm_mon + 1, current_time->tm_mday, current_time->tm_hour,
                        current_time->tm_min, current_time->tm_sec);
    
    char buffer[128];

    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s D %s %3d %s %ld\n",
            plid, player->current_game->secret_key,
            player->current_game->max_time, time_str,
            player->current_game->start_time);
    fwrite(buffer, 1, strlen(buffer), fp);
    fclose(fp);

    mutex_unlock(plid_mutex);
    send_udp_response("RDB OK\n", client_addr, client_addr_len, udp_socket);
}