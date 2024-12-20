#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_debug(const char *input) {
    char plid[MAX_COMMAND_SIZE];
    unsigned int max_playtime;
    char code[2*MAX_COLORS];
    memset(plid, 0, sizeof(plid));
    memset(code, 0, sizeof(code));
    code[2*MAX_COLORS-1] = '\0';
    code[1] = ' ';
    code[3] = ' ';
    code[5] = ' ';

    char extra;
    if (sscanf(input, "debug %s %u %c %c %c %c %c", plid, &max_playtime, &code[0], &code[2], &code[4], &code[6], &extra) != 6) {
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

    char message[SMALL_BUFFER];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "DBG %s %03d %s\n", plid, time, code);

    char response[SMALL_BUFFER];
    memset(response, 0, sizeof(response));

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        return error_communicating_with_server(EMPTY);
    }

    receive_debug_msg(plid, response);
}

void receive_debug_msg(const char *plid, const char *response) {
    char status[SMALL_BUFFER];
    memset(status, 0, sizeof(status));

    if (sscanf(response, "RDB %s", status) != 1) {
        return error_communicating_with_server(response);
    }
    if (strcmp(status, "OK") == 0) {
        currTries = 0;
        currPlayer = 1;
        setPLID = 0;
        hasStarted = 1;
        strcpy(plidCurr, plid);
        printf("Debug game started. You can begin playing.\n");
    } else if (strcmp(status, "NOK") == 0) {
        printf("Debug game not started: an ongoing game exists for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error starting debug game. Please check your inputs or try again later.\n");
    } else {
        return error_communicating_with_server(response);
    }
}