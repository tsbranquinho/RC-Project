#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_help(const char *input) {
    if (strlen(input) != 4) {
        printf("Invalid command format\n");
        return TRUE;
    }
    printf("Commands:\n");
    printf("\t• start <plid> <time>\n");
    printf("\t• try <code>\n");
    printf("\t• quit\n");
    printf("\t• exit\n");
    printf("\t• debug <plid> <time> <code>\n");
    printf("\t• show_trials\n");
    printf("\t• scoreboard\n");
    printf("\t• set <plid>\n");
    printf("\t• sleep <time>\n");
    printf("\t• clean\n");
    printf("\t• hint\n");
    printf("\t• help\n");
    return TRUE;
}