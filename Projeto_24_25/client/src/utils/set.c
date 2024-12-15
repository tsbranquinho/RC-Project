#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_set(const char* input) {
    char plid[MAX_COMMAND_SIZE];
    memset(plid, 0, sizeof(plid));
    if (sscanf(input, "set %s", plid) != 1 ) {
        return invalid_command_format(CMD_SET);
    }
    if (strlen(plid) != ID_SIZE) {
        invalid_player_id(plid);
        return TRUE;
    }
    if (errorCurrentPlayer(plid)) {
        return TRUE;
    }
    strcpy(plidCurr, plid);
    currPlayer = TRUE;
    setPLID = TRUE;
    return TRUE;
}