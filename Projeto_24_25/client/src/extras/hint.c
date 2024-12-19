#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_hint(const char *input) {
    if (sscanf(input, "hint") != 0) {
        return invalid_command_format(CMD_HINT);
    }
    if (currPlayer == 0) {
        error_no_game(CMD_HINT);
        return TRUE;	
    }
    char message[SMALL_BUFFER];
    memset(message, 0, sizeof(message));
    hint++;
    if (hint > 4) {
        printf("You have already used all the hints.\n");
        return TRUE;
    }
    sprintf(message, "HNT %s %d\n", plidCurr, hint);
    char response[SMALL_BUFFER];
    memset(response, 0, sizeof(response));
    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        error_communicating_with_server(EMPTY);
        return TRUE;
    }
    char status[MAX_STATUS_SIZE];
    char c1, c2, c3, c4;
    memset(status, 0, sizeof(status));
    if (sscanf(response, "RHT %s %c %c %c %c", status, &c1, &c2, &c3, &c4) < 1) {
        error_communicating_with_server(response);
        return TRUE;
    }
    if (strcmp(status, "OK") == 0 && (strchr(COLOR_OPTIONS, c1) != NULL || c1 == '?') && (strchr(COLOR_OPTIONS, c2) != NULL || c2 == '?') && (strchr(COLOR_OPTIONS, c3) != NULL || c3 == '?') && (strchr(COLOR_OPTIONS, c4) != NULL || c4 == '?')) {
        printf("Hint: %c %c %c %c\n", c1, c2, c3, c4);
    } else if (strcmp(status, "NOK") == 0) {
        printf("No ongoing game found for this player.\n");
    } else if(strcmp(status, "INV") == 0) {
        printf("Invalid hint request. Please try again.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error getting hint. Please verify your inputs or try again later.\n");
    } else {
        error_communicating_with_server(response);
    }
    return TRUE;

}