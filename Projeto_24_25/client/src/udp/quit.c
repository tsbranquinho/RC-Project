#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_quit(const char *input) {
    if (strcmp(input,"quit") != 0) {
        return invalid_command_format(CMD_QUIT);
    }
    quit_game();
    return TRUE;
}

void quit_game() {

    if (hasStarted == 0) {
        return error_no_game(CMD_QUIT);
    }

    char message[SMALL_BUFFER];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "QUT %s\n", plidCurr);

    char response[SMALL_BUFFER];
    memset(response, 0, sizeof(response));

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        return error_communicating_with_server(EMPTY);
    }

    receive_quit_msg(response);
}

void receive_quit_msg(const char *response) {
    char status[SMALL_BUFFER];
    memset(status, 0, sizeof(status));
    char secret_code[2*MAX_COLORS];
    memset(secret_code, 0, sizeof(secret_code));

    char extra;
    int matched = sscanf(response, "RQT %s %c %c %c %c %c", status, &secret_code[0], &secret_code[2], &secret_code[4], &secret_code[6], &extra);

    if (matched < 1 || matched > 5) {
        return error_communicating_with_server(response);
    }

    if (strcmp(status, "OK") == 0 && matched == 5) {
        printf("Game successfully quit. The secret code was: %s\n", secret_code);
        hasStarted = 0;
    } else if (strcmp(status, "NOK") == 0) {
        printf("No ongoing game found for this player.\n");
        hasStarted = 0;
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error quitting game. Please verify your inputs or try again later.\n");
    } else {
        return error_communicating_with_server(response);
    }

    end_game();
}

void end_game() {
    currPlayer = 0;
    currTries = 0;
}