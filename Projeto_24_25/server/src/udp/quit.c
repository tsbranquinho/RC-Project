#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_quit_request(char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];

    char extra;
    if (sscanf(request, "QUT %6s %c", plid, &extra) != 1) {

        send_udp_response("RQT ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    if(!valid_plid(plid)) {
        send_udp_response("RQT ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    Player *player = find_player(plid);
    if (!player) {
        send_udp_response("RQT NOK\n", client_addr, client_addr_len, udp_socket);
        set_verbose_quit_message(request, plid);
        return SUCCESS;
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    char response[SMALL_BUFFER];
    char temp[2*MAX_COLORS];
    convert_code(temp, player->current_game->secret_key, SECRET_TO_CODE);
    player->current_game->end_status = 'Q';
    snprintf(response, sizeof(response), "RQT OK %s\n", temp);
    if (end_game(player, plid_mutex) == -1) {
        send_udp_response("RQT ERR\n", client_addr, client_addr_len, udp_socket);
        return ERROR;
    }

    send_udp_response(response, client_addr, client_addr_len, udp_socket);
    set_verbose_quit_message(request, plid);
    return SUCCESS;
}

int end_game(Player *player, pthread_mutex_t *plid_mutex) {
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

    char time_str[20]; // YYYY-MM-DD HH:MM:SS

    sprintf(time_str, "%04d-%02d-%02d %02d:%02d:%02d", current_time->tm_year + 1900, current_time->tm_mon + 1, 
        current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);

    char buffer[SMALL_BUFFER];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s %ld\n", time_str, last_time-player->current_game->start_time);

    fwrite(buffer, 1, strlen(buffer), file);
    fclose(file);

    char directory[13]; // GAMES/ + PLID
    memset(directory, 0, sizeof(directory));

    snprintf(directory, sizeof(directory), "GAMES/%s", player->plid);
    if (mkdir(directory, 0777) == -1 && errno != EEXIST) {
        perror("Error creating directory");
        mutex_unlock(plid_mutex);
        return -1;
    }

    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "GAMES/%s/%04d%02d%02d_%02d%02d%02d_%c.txt", player->plid,
        current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday,
        current_time->tm_hour, current_time->tm_min, current_time->tm_sec, player->current_game->end_status);

    if (rename(player->current_game->filename, buffer) == -1) {
        perror("Error moving file");
        mutex_unlock(plid_mutex);
        return -1;
    }

    remove_player(player->plid, plid_mutex);
    return 0;
}

int score_game(Player *player) {
    int score = 1000;
    score -= (player->current_game->trial_count - 1) * 100;
    score -= (player->current_game->last_time - player->current_game->start_time) / 3;
    score -= player->current_game->hint_count * 125;
    if (score < 1) {
        score = 1;
    }
    if (score > 999) {
        score = 999;
    }

    struct tm *current_time = gmtime(&player->current_game->last_time);
    char mode[6];
    memset(mode, 0, sizeof(mode));
    if (player->current_game->mode == DEBUG) {
        strncpy(mode, "DEBUG", sizeof(mode));
    }
    else {
        strncpy(mode, "PLAY", sizeof(mode));
    }

    pthread_rwlock_wrlock(&scoreboard_lock);

    char filename[FILENAME_SIZE];
    memset(filename, 0, sizeof(filename));
    snprintf(filename, sizeof(filename), "SCORES/%03d_%s_%02d%02d%04d_%02d%02d%02d.txt", 
        score, player->plid, current_time->tm_mday, current_time->tm_mon + 1, current_time->tm_year + 1900,
        current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("fopen");
        pthread_rwlock_unlock(&scoreboard_lock);
        return -1;
    }

    char buffer[SMALL_BUFFER];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%03d %s %s %d %s\n",
        score, player->plid, player->current_game->secret_key, player->current_game->trial_count, mode);
    if (fwrite(buffer, 1, strlen(buffer), file) != strlen(buffer)) {
        perror("fwrite");
        fclose(file);
        pthread_rwlock_unlock(&scoreboard_lock);
        return -1;
    }
    fclose(file);

    pthread_rwlock_unlock(&scoreboard_lock);
    return 0;
}

void set_verbose_quit_message(char* request, const char* plid) {
    memset(request, 0, SMALL_BUFFER);
    sprintf(request, "Quit request: PLID = %s\n", plid);
}