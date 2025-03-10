#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_sleep(const char* input) {
    int time;
    if (sscanf(input, "sleep %d", &time) != 1) {
        return invalid_command_format(CMD_SLEEP);
    }
    if (time <= 0) {
        fprintf(stderr, "Invalid time\n");
        return TRUE;
    }
    sleep(time);
    return TRUE;
}