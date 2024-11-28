#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

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
    char temp[2*MAX_COLORS];
    convert_code(temp, player->current_game->secret_key, SECRET_TO_CODE);
    snprintf(response, sizeof(response), "RQT OK %s\n", temp);
    for (Trials *trial = player->current_game->trial; trial != NULL; trial = trial->prev) {
        free(trial);
    }
    free(player->current_game);
    player->is_playing = 0;
    player->current_game = NULL;

    send_udp_response(response, client_addr, client_addr_len, udp_socket);
}