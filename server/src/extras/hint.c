#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_hint_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE+1];
    int hint;
    memset(plid, 0, sizeof(plid));
    if (sscanf(request, "HNT %s %d", plid, &hint) != 2) {
        send_udp_response("RHT ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    Player *player = find_player(plid);

    if (!player) {
        send_udp_response("RHT NOK\n", client_addr, client_addr_len, udp_socket);
        set_verbose_hint_message(request, plid, hint);
        return SUCCESS;
    }
    if (++player->current_game->hint_count != hint) {
        send_udp_response("RHT INV\n", client_addr, client_addr_len, udp_socket);
        set_verbose_hint_message(request, plid, hint);
        return SUCCESS;
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    char response[SMALL_BUFFER];
    memset(response, 0, sizeof(response));
    switch (hint) {
        case 1:
            sprintf(response, "RHT OK %c ? ? ?\n", player->current_game->secret_key[0]);
            break;
        case 2:
            sprintf(response, "RHT OK %c %c ? ?\n", player->current_game->secret_key[0], player->current_game->secret_key[1]);
            break;
        case 3:
            sprintf(response, "RHT OK %c %c %c ?\n", player->current_game->secret_key[0], player->current_game->secret_key[1], player->current_game->secret_key[2]);
            break;
        case 4:
            sprintf(response, "RHT OK %c %c %c %c\n", player->current_game->secret_key[0], player->current_game->secret_key[1], player->current_game->secret_key[2], player->current_game->secret_key[3]);
            break;
        default:
            send_udp_response("RHT ERR\n", client_addr, client_addr_len, udp_socket);
            mutex_unlock(plid_mutex);
            return ERROR;
    }
    mutex_unlock(plid_mutex);
    send_udp_response(response, client_addr, client_addr_len, udp_socket);
    set_verbose_hint_message(request, plid, hint);
    return SUCCESS;
}

void set_verbose_hint_message(char *request, const char *plid, int hint) {
    memset(request, 0, strlen(request));
    sprintf(request, "Hint request: PLID = %s, hint = %d\n", plid, hint);
}