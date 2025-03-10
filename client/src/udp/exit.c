#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_exit(const char *input) {
    if (strcmp(input,"exit") != 0) {
        return invalid_command_format(CMD_EXIT);
    }

    exit_game();
    return EXIT_COMMAND;
}

void exit_game() {
    if (currPlayer && hasStarted) {
        quit_game();
    }
    printf("Exiting the application.\n");
}