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
    player->current_game->end_status = 'Q';
    snprintf(response, sizeof(response), "RQT OK %s\n", temp);
    if (end_game(player) == -1) {
        send_udp_response("RQT ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    send_udp_response(response, client_addr, client_addr_len, udp_socket);
}

int end_game(Player *player) {
    FILE *file = fopen(player->current_game->filename, "a");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }
    time_t last_time;
    if (player->current_game->end_status == 'Q') {
        time(&last_time);
    }
    else {
        last_time = player->current_game->last_time;
    }
    struct tm *current_time = gmtime(&last_time);
    char time_str[20];
    sprintf(time_str, "%04d-%02d-%02d %02d:%02d:%02d", current_time->tm_year + 1900, current_time->tm_mon + 1, 
        current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s %ld\n", time_str, last_time-player->current_game->start_time);
    fwrite(buffer, 1, strlen(buffer), file);
    fclose(file);
    // Rename the file
    //GAMES/XXXXXX
    char directory[13];
    memset(directory, 0, sizeof(directory));
    snprintf(directory, sizeof(directory), "GAMES/%s", player->plid);
    if (mkdir(directory, 0777) == -1 && errno != EEXIST) {
        perror("Error creating directory");
        return -1;
    }
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "GAMES/%s/%04d%02d%02d_%02d%02d%02d_%c.txt", player->plid,
        current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday,
        current_time->tm_hour, current_time->tm_min, current_time->tm_sec, player->current_game->end_status);
    if (rename(player->current_game->filename, buffer) == -1) {
        perror("Error moving file");
        return -1;
    }

    for (Trials *trial = player->current_game->trial; trial != NULL; trial = trial->prev) {
        free(trial);
    }
    free(player->current_game);
    player->is_playing = 0;
    player->current_game = NULL;
    return 0;
}