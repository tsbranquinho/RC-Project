#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void usage(const char *progname) {
    fprintf(stderr, "Usage: %s [-p GSport] [-v]\n", progname);
    fprintf(stderr, "  -p GSport  Specify the port number (only one) of the game server (optional).\n");
    fprintf(stderr, "  -v         Enable verbose mode (optional).\n");
}
