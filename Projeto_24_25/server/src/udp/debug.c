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