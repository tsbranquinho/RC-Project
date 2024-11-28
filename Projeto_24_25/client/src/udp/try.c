#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_try(const char *input) {
    char code[8];
    memset(code, 0, sizeof(code));
    if (sscanf(input, "try %[^\n]", code) != 1) {
        return invalid_command_format(CMD_TRY);
    }
    try_code(code);
    return FALSE;
}

void try_code(const char *code) {
    char trimmed_code[2*MAX_COLORS];
    printf("Code: %s\n", code);
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
        printf("trimmed_code[%d]: %c\n", i, trimmed_code[i]);
        printf("trimmed_code[%d]: %c\n", i+1, trimmed_code[i+1]);
        if (strchr(COLOR_OPTIONS, trimmed_code[i]) == NULL) {
            return invalid_code(COLOR);
        }
        if (trimmed_code[i+1] != ' ' && trimmed_code[i+1] != '\0') {
            return invalid_code(SPACE);
        }
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "TRY %s %s %d\n",plidCurr, trimmed_code, currTries+1);

    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    printf("[DEBUG] Sending try request: %s", message);

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        return error_communicating_with_server(EMPTY);
    }

    printf("[DEBUG] Received response: %s", response);
    receive_try_msg(response);
}

void receive_try_msg(const char *response) {
    char status[BUFFER_SIZE];
    int black = 0, white = 0;
    char code[2 * MAX_COLORS];
    int tries;
    memset(status, 0, sizeof(status));
    memset(code, 0, sizeof(code));

    if (sscanf(response, "RTR %s %d %d %d %[^\n]", status, &tries, &black, &white, code) < 1) {
        return error_communicating_with_server(response);
    }
    code[2*MAX_COLORS] = '\0';
    printf("check\n");
    printf("status: %s\n", status);

    //TODO: verificar para cada resposta de status
    //    : verificar se o code está bem mandado

    if (strcmp(status, "OK") == 0) {
        currTries = tries;
        printf("Tries: %d, Black: %d, White: %d\n", currTries, black, white);

        if (black == MAX_COLORS) {
            printf("Congratulations! You've cracked the secret code.\n");
        }
    } else if (strcmp(status, "DUP") == 0) {
        printf("Duplicate code entered. Try a different combination.\n");
    } else if (strcmp(status, "ENT") == 0) {
        printf("No more attempts left. You lose! The secret code was: %s\n", code);
    } else if (strcmp(status, "ETM") == 0) {
        printf("Time limit exceeded. You lose! The secret code was: %s\n", code);
    } else if (strcmp(status, "INV") == 0) {
        printf("Invalid trial format or sequence.\n");
    } else if (strcmp(status, "NOK") == 0) {
        printf("No ongoing game found for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error trying code. Please verify inputs or try again later.\n");
    } else {
        return error_communicating_with_server(response);
    }
}