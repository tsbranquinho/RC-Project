#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

Command get_next_command(char *input) {
    if (strncmp(input, "start", 5) == 0) return CMD_START;
    if (strncmp(input, "try", 3) == 0) return CMD_TRY;
    if (strncmp(input, "quit", 4) == 0) return CMD_QUIT;
    if (strncmp(input, "exit", 4) == 0) return CMD_EXIT;
    if (strncmp(input, "debug", 5) == 0) return CMD_DEBUG;
    if (strncmp(input, "st", 2) == 0 || strncmp(input, "show_trials", 11) == 0) return CMD_SHOW_TRIALS;
    if (strncmp(input, "sb", 2) == 0 || strncmp(input, "scoreboard", 10) == 0) return CMD_SCOREBOARD;
    if (strncmp(input, "set", 3) == 0) return CMD_SET;
    if (strncmp(input, "sleep", 5) == 0) return CMD_SLEEP;
    if (strncmp(input, "clean", 5) == 0) return CMD_CLEAN;
    if (strncmp(input, "hint", 4) == 0) return CMD_HINT;
    if (strncmp(input, "help", 4) == 0) return CMD_HELP;
    return CMD_INVALID;
}

int handle_command(Command cmd, const char* input) {
    int return_value = TRUE;
    switch (cmd) {
        case CMD_START:
            return_value = handle_start(input);
            break;
        
        case CMD_TRY: 
            return_value = handle_try(input);
            break;
        
        case CMD_QUIT: 
            return_value = handle_quit(input);
            break;
        
        case CMD_EXIT: 
            return_value = handle_exit(input);
            if (return_value == EXIT_COMMAND) {
                return EXIT_COMMAND;
            }
            break;
        
        case CMD_DEBUG:
            return_value = handle_debug(input);
            break;

        case CMD_SHOW_TRIALS:
            return_value = handle_show_trials(input);
            break;

        case CMD_SCOREBOARD:
            return_value = handle_show_scoreboard(input);
            break;

        case CMD_SET:
            return_value = handle_set(input);
            break;

        case CMD_SLEEP:
            return_value = handle_sleep(input);
            break;

        case CMD_CLEAN:
            return_value = handle_clean(input);
            break;

        case CMD_HINT:
            return_value = handle_hint(input);
            break;

        case CMD_HELP:
            return_value = handle_help(input);
            break;
        default:
            invalid_command_format(CMD_INVALID);
    }
    return return_value;
}