#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_try(const char *input) {

    char code[2*MAX_COLORS];
    memset(code, 0, sizeof(code));
    code[2*MAX_COLORS-1] = '\0';
    code[1] = ' ';
    code[3] = ' ';
    code[5] = ' ';

    char extra;
    if (sscanf(input, "try %c %c %c %c %c", &code[0], &code[2], &code[4], &code[6], &extra) != 4) {
        return invalid_command_format(CMD_TRY);
    }
    try_code(code);
    return TRUE;
}

void try_code(const char *code) {
    char trimmed_code[2*MAX_COLORS];
    memset(trimmed_code, 0, sizeof(trimmed_code));
    snprintf(trimmed_code, sizeof(trimmed_code), "%s", code);

    trimmed_code[2*MAX_COLORS-1] = '\0';

    if (currPlayer == 0) {
        return error_no_game(CMD_TRY);
    }

    if (strlen(trimmed_code) != 2*MAX_COLORS - 1) {
        return invalid_code(LENGTH);
    }

    for (int i = 0; i < 2*MAX_COLORS; i += 2) {
        if (strchr(COLOR_OPTIONS, trimmed_code[i]) == NULL) {
            return invalid_code(COLOR);
        }
        if (trimmed_code[i+1] != ' ' && trimmed_code[i+1] != '\0') {
            return invalid_code(SPACE);
        }
    }

    char message[SMALL_BUFFER];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "TRY %s %s %d\n",plidCurr, trimmed_code, currTries+1);

    char response[SMALL_BUFFER];
    memset(response, 0, sizeof(response));

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        return error_communicating_with_server(EMPTY);
    }

    receive_try_msg(response);
}


void receive_try_msg(const char *response) {
    char status[MAX_STATUS_SIZE];
    int black = 0, white = 0;
    char code[2 * MAX_COLORS];
    int tries;
    memset(status, 0, sizeof(status));
    memset(code, 0, sizeof(code));

    code[2*MAX_COLORS-1] = '\0';
    code[1] = ' ';
    code[3] = ' ';
    code[5] = ' ';
    char extra;


    if (sscanf(response, "RTR %s", status) != 1) {
        return error_communicating_with_server(response);
    }

    if (strcmp("OK", status) == 0) {

        if (sscanf(response + 4 + 3, "%d %d %d %c", &tries, &black, &white, &extra) != 3) {
            return error_communicating_with_server(response);
        }
        currTries = tries;

        printf("Tries: %d, Black: %d, White: %d\n", currTries, black, white);

        if (black == MAX_COLORS) {
            printf("Congratulations! You've cracked the secret code.\n");

            currTries = 0;
            hasStarted = 0;
        }

    } else if (strcmp("DUP", status) == 0) {
        printf("Duplicate code entered. Try a different combination.\n");
    } else if (strcmp("INV", status) == 0) {
        printf("Invalid trial format or sequence.\n");
    } else if (strcmp("NOK", status) == 0) {
        printf("No ongoing game found for this player.\n");
    } else if (strcmp("ERR", status) == 0) {
        printf("Error trying code. Please verify inputs or try again later.\n");
    } else if (strcmp("ENT", status) == 0) {
        if (sscanf(response + 4 + 4, "%c %c %c %c %c", &code[0], &code[2], &code[4], &code[6], &extra) != 4) {
            return error_communicating_with_server(response);
        }
        printf("No more attempts left. You lose! The secret code was: %s\n", code);
        currTries = 0;

    } else if (strcmp("ETM", status) == 0) {
        if (sscanf(response + 4 + 4, "%c %c %c %c %c", &code[0], &code[2], &code[4], &code[6], &extra) != 4) {
            return error_communicating_with_server(response);
        }
        printf("Time limit exceeded. You lose! The secret code was: %s\n", code);
        currTries = 0;
    } else {
        return error_communicating_with_server(response);
    }

    
}