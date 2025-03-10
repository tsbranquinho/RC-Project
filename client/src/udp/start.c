#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_start(const char *input) {
    char plid[MAX_COMMAND_SIZE];
    memset(plid, 0, sizeof(plid));
    int time;

    char extra;
    if (sscanf(input, "start %s %d %c", plid, &time, &extra) != 2 ) {
        return invalid_command_format(CMD_START);
    }

    if(errorCurrentPlayer(plid)){
        return TRUE;
    }

    start_game(plid, time);
    return TRUE;
}

void start_game(const char *plid, unsigned int time) {

    if (strlen(plid) != ID_SIZE || !is_number(plid)) {
        return invalid_player_id(plid);
    }
    if (time < 0 || time > MAX_PLAYTIME) {
        return invalid_playtime(time);
    }

    char message[SMALL_BUFFER];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "SNG %s %03d\n", plid, time);

    char response[SMALL_BUFFER];
    memset(response, 0, sizeof(response));

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        return error_communicating_with_server(EMPTY);
    }

    receive_start_msg(response, plid);
}

void receive_start_msg(const char *response, const char *plid) {
    char status[SMALL_BUFFER];
    memset(status, 0, sizeof(status));

    char extra;
    if (sscanf(response, "RSG %s %c", status,&extra) != 1) {
        return error_communicating_with_server(response);
    }

    if (strcmp(status, "OK") == 0) {
        printf("Game started successfully! You can begin playing.\n");
        strcpy(plidCurr, plid);
        currTries = 0;
        currPlayer = 1;
        setPLID = 0;
        hasStarted = 1;
    } else if (strcmp(status, "NOK") == 0) {
        printf("Game not started: an ongoing game exists for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error starting game. Please check your inputs or try again later.\n");
    } else {
        return error_communicating_with_server(response);
    }
}