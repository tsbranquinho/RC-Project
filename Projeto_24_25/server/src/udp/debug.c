#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_debug_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];
    int max_time;
    char key[MAX_COLORS + 1];

    if (sscanf(request, "DBG %6s %3d %4s", plid, &max_time, key) != 3 || max_time <= 0 || max_time > MAX_PLAYTIME) {
        send_udp_response("RDB ERR\n", client_addr, client_addr_len, udp_socket);
        return;
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
    strncpy(player->current_game->secret_key, key, MAX_COLORS);

    //TODO create file

    mutex_unlock(plid_mutex);
    send_udp_response("RDB OK\n", client_addr, client_addr_len, udp_socket);
}