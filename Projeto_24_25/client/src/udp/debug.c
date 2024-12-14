#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_debug(const char *input) {
    char plid[7];
    unsigned int max_playtime;
    char code[2*MAX_COLORS];
    memset(plid, 0, sizeof(plid));
    memset(code, 0, sizeof(code));

    //TODO verificar se isto está bem
    if (sscanf(input, "debug %s %u %[^\n]", plid, &max_playtime, code) != 3) {
        return invalid_command_format(CMD_DEBUG);
    }

    if(errorCurrentPlayer(plid)){
        return TRUE;
    }

    debug_game(plid, max_playtime, code);
    return TRUE;
}

void debug_game(const char *plid, unsigned int time, const char *code) {
    if (strlen(plid) != ID_SIZE || !is_number(plid)) {
        return invalid_player_id(plid);
    }

    if (time < 0 || time > MAX_PLAYTIME) {
        return invalid_playtime(time);
    }

    if (strlen(code) != 2*MAX_COLORS-1) {
        return invalid_code(LENGTH);
    }

    for (int i = 0; i < MAX_COLORS; i++) {
        if (strchr(COLOR_OPTIONS, code[2*i]) == NULL) {
            return invalid_code(COLOR);
        }
        if (code[2*i+1] != ' ' && code[2*i+1] != '\0') {
            return invalid_code(SPACE);
        }
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "DBG %s %03d %s\n", plid, time, code); // Time padded to 3 digits

    char response[BUFFER_SIZE];
    printf("[DEBUG] Sending debug game request: %s", message);

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        return error_communicating_with_server(EMPTY);
    }

    printf("[DEBUG] Received response: %s", response);
    receive_debug_msg(plid, response);
}

void receive_debug_msg(const char *plid, const char *response) {
    char status[BUFFER_SIZE];
    memset(status, 0, sizeof(status));

    if (sscanf(response, "RDB %s", status) != 1) {
        return error_communicating_with_server(response);
    }
    if (strcmp(status, "OK") == 0) {
        printf("Debug game started successfully! You can begin playing.\n");
        currPlayer = 1;
        currTries = 0;
        strcpy(plidCurr, plid);
    } else if (strcmp(status, "NOK") == 0) {
        printf("Debug game not started: an ongoing game exists for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error starting debug game. Please check your inputs or try again later.\n");
    } else {
        return error_communicating_with_server(response);
    }
}