#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void start_game(const char *plid, unsigned int time) {

    if (strlen(plid) != ID_SIZE || !is_number(plid)) {
        fprintf(stderr, "Invalid player ID, must be 6 numerical digits.\n");
        return;
    }
    if (time < 0 || time > MAX_PLAYTIME) {
        fprintf(stderr, "Invalid playtime, must be between 0 and 600 seconds.\n");
        return;
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "SNG %s %03d\n", plid, time);

    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    printf("[DEBUG] Sending start game request: %s", message); // Debug log

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        fprintf(stderr, "Error communicating with the server.\n");
        return;
    }

    printf("[DEBUG] Received response: %s", response); // Debug log
    receive_start_msg(response, plid);
}

void receive_start_msg(const char *response, const char *plid) {
    char status[BUFFER_SIZE];
    memset(status, 0, sizeof(status));
    if (sscanf(response, "RSG %s", status) != 1) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
    }

    if (strcmp(status, "OK") == 0) {
        printf("Game started successfully! You can begin playing.\n");
        strcpy(plidCurr, plid);
        currTries = 0;
        currPlayer = 1;
    } else if (strcmp(status, "NOK") == 0) {
        printf("Game not started: an ongoing game exists for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error starting game. Please check your inputs or try again later.\n");
    } else {
        fprintf(stderr, "Unexpected server response: %s\n", status);
    }
}