#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_exit(const char *input) {
    if (sscanf(input, "exit") != 0) {
        return invalid_command_format(CMD_EXIT);
    }

    exit_game();
    return EXIT_COMMAND;
}

void exit_game() {
    // If the player is playing, quit the game
    if (currPlayer) {
        quit_game();
    }
    printf("Exiting the application.\n");
}