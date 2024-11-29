#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_start_request(const char *request, struct sockaddr_in *client_addr, socklen_t client_addr_len, int udp_socket) {
    char plid[ID_SIZE + 1];
    int max_time;
    printf("Got a start request\n");

    if (sscanf(request, "SNG %6s %3d", plid, &max_time) != 2 || max_time <= 0 || max_time > MAX_PLAYTIME) {
        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }
    
    Player *player = find_player(plid);
    if (player != NULL && player->is_playing) {
        send_udp_response("RSG NOK\n", client_addr, client_addr_len, udp_socket);
        return;
    }

    if (player == NULL) {
        player = create_player(plid);
        insert_player(player);
    }

    player->is_playing = 1;
    player->current_game = malloc(sizeof(Game));
    time(&player->current_game->start_time);
    player->current_game->trial_count = 0;
    player->current_game->max_time = max_time;
    generate_random_key(player->current_game->secret_key);
    player->current_game->trial = NULL;
    player->current_game->mode = PLAY;
    //GAME_XXXXXX.txt
    char filename[GAME_FILE_NAME_SIZE+6]; // 6 for "GAMES/"
    snprintf(filename, sizeof(filename), "GAMES/GAME_%s.txt", plid);
    strncpy(player->current_game->filename, filename, sizeof(player->current_game->filename));
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Error creating file");
        send_udp_response("RSG ERR\n", client_addr, client_addr_len, udp_socket);
        return;
    }
    char time_str[20];
    struct tm *current_time = gmtime(&player->current_game->start_time);
    sprintf(time_str, "%4d-%02d-%02d %02d:%02d:%02d", current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s P %s %3d %s %ld\n",
            plid, player->current_game->secret_key, 
            player->current_game->max_time, time_str, 
            player->current_game->start_time);
    fwrite(buffer, 1, strlen(buffer), fp);
    fclose(fp);

    send_udp_response("RSG OK\n", client_addr, client_addr_len, udp_socket);
}

void generate_random_key(char *key) {
    const char *colors = COLOR_OPTIONS;
    srand(time(NULL));
    for (int i = 0; i < MAX_COLORS; i++) {
        key[i] = colors[rand() % strlen(colors)];
    }
    key[MAX_COLORS] = '\0';
}