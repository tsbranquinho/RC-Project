#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void debug_game(const char *plid, unsigned int time, const char *code) {
    if (strlen(plid) != ID_SIZE || !is_number(plid)) {
        fprintf(stderr, "Invalid player ID. It must be 6 numerical digits.\n");
        return;
    }

    if (time < 0 || time > MAX_PLAYTIME) {
        fprintf(stderr, "Invalid playtime. It must be between 0 and 600 seconds.\n");
        return;
    }

    if (strlen(code) != MAX_COLORS) {
        fprintf(stderr, "Invalid code length. It must be exactly 4 characters.\n");
        return;
    }

    for (int i = 0; i < MAX_COLORS; i++) {
        if (strchr(COLOR_OPTIONS, code[i]) == NULL) {
            fprintf(stderr, "Invalid color code. Use only R, G, B, Y, O, or P.\n");
            return;
        }
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "DBG %s %03d %s\n", plid, time, code); // Time padded to 3 digits

    char response[BUFFER_SIZE];
    printf("[DEBUG] Sending debug game request: %s", message);

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        fprintf(stderr, "Error communicating with the server.\n");
        return;
    }

    printf("[DEBUG] Received response: %s", response);
    receive_debug_msg(plid, response);
}

void receive_debug_msg(const char *plid, const char *response) {
    char status[BUFFER_SIZE];
    memset(status, 0, sizeof(status));

    if (sscanf(response, "RDB %s", status) != 1) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
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
        fprintf(stderr, "Unexpected server response: %s\n", status);
    }
}