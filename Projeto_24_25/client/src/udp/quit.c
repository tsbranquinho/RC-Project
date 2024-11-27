#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void quit_game() {

    if (currPlayer == 0) {
        printf("Successfully quit the game.\n");
        return;
    }

    char message[BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    snprintf(message, sizeof(message), "QUT %s\n", plidCurr);

    char response[BUFFER_SIZE];
    printf("[DEBUG] Sending quit request: %s", message); // Debug log

    if (send_udp_skt(message, response, sizeof(response), GSIP, GSport) < 0) {
        fprintf(stderr, "Error communicating with the server.\n");
        return;
    }

    printf("[DEBUG] Received response: %s", response); // Debug log
    receive_quit_msg(response);
}

void receive_quit_msg(const char *response) {
    char status[BUFFER_SIZE];
    memset(status, 0, sizeof(status));
    char secret_code[2*MAX_COLORS];
    memset(secret_code, 0, sizeof(secret_code));

    int matched = sscanf(response, "RQT %s %[^\n]", status, secret_code);
    if (matched < 1) {
        fprintf(stderr, "Invalid response from server: %s\n", response);
        return;
    }

    if (strcmp(status, "OK") == 0 && matched == 2) {
        printf("Game successfully quit. The secret code was: %s\n", secret_code);
    } else if (strcmp(status, "NOK") == 0) {
        printf("No ongoing game found for this player.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Error quitting game. Please verify your inputs or try again later.\n");
    } else {
        fprintf(stderr, "Unexpected server response: %s\n", status);
    }

    end_game();
}

void end_game() {
    currPlayer = 0;
    currTries = 0;
    plidCurr[0] = '\0';
    // temos que dar flush do que tiver nos buffers
    //TODO
}