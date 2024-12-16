#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int errorCurrentPlayer(const char *plid) {
    if(currPlayer && currTries != 0 && strcmp(plidCurr, plid) != 0 && setPLID == 0){
        printf("There is already a player playing. Please quit the current game to start a new one.\n");
        return 1;
    }
    if(strlen(plid) == 0){
        printf("No player ID provided.\n");
        return 1;
    }
    return 0;
}

void sig_detected(int sig) {
    if (currPlayer) {
        printf("Game interrupted. Exiting...\n");
        //exit_game(); //TODO só no final podemos tirar isto senão semrpe que fizermos ctrl c o server tem estar ligado e o crlh
        exit(0);
    } else {
        printf("Exiting...\n");
        //exit_game();    //TODO só no final podemos tirar isto senão semrpe que fizermos ctrl c o server tem estar ligado e o crlh
        exit(0);
    }
}

void error_communicating_with_server(const char *msg) {
    if (strlen(msg) > 0) {
        fprintf(stderr, "Unexpected server response: %s", msg);
    }
    else {
        fprintf(stderr, "Error communicating with the server.\n");
    }
}

int invalid_command_format(Command cmd) {
    if (cmd == CMD_START) {
        fprintf(stderr, "Invalid 'start' command format. Format: start <player_id> <playtime>\n");
    } else if (cmd == CMD_TRY) {
        fprintf(stderr, "Invalid 'try' command format. Format: try C1 C2 C3 C4\n");
    } else if (cmd == CMD_QUIT) {
        fprintf(stderr, "Invalid 'quit' command format. Format: quit\n");
    } else if (cmd == CMD_EXIT) {
        fprintf(stderr, "Invalid 'exit' command format. Format: exit\n");
    } else if (cmd == CMD_DEBUG) {
        fprintf(stderr, "Invalid 'debug' command format. Format: debug <player_id> <playtime> C1 C2 C3 C4\n");
    } else if (cmd == CMD_SHOW_TRIALS) {
        fprintf(stderr, "Invalid 'show_trials' command format. Format: st or Format: show_trials\n");
    } else if (cmd == CMD_SCOREBOARD) {
        fprintf(stderr, "Invalid 'scoreboard' command format. Format: sb or Format: scoreboard\n");
    //extras
    } else if (cmd == CMD_SET){
        fprintf(stderr, "Invalid 'set' command format. Format: set <player_id>\n");
    } else {
        fprintf(stderr, "Invalid command format.\n");
    }
    return TRUE;
}

void invalid_player_id(const char *plid) {
    fprintf(stderr, "Invalid player ID: %s. It must be 6 numerical digits.\n", plid);
}

void invalid_playtime(unsigned int time) {
    fprintf(stderr, "Invalid playtime: %d. It must be between 0 and 600 seconds.\n", time);
}

void invalid_code(int mode) {
    if (mode == LENGTH) {
        fprintf(stderr, "Invalid code length. It must be exactly 7 characters.\n"); //TODO devemos ser mais permissivos
    } else if (mode == COLOR) {
        fprintf(stderr, "Invalid color code. Use only R, G, B, Y, O, or P.\n");
    } else if (mode == SPACE) {
        fprintf(stderr, "Invalid code format. Use spacing.\n");
    }
}

void error_no_game(Command cmd) {
    if (cmd == CMD_TRY) {
        printf("No game started. Please start a game before trying a code.\n");
    } else if (cmd == CMD_QUIT) {
        printf("No game to quit.\n");
    }
}

