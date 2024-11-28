#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void usage(const char *progname) {
    fprintf(stderr, "Usage: %s [-n GSIP] [-p GSport]\n", progname);
    fprintf(stderr, "  -n GSIP    Specify the IP address of the game server (optional).\n");
    fprintf(stderr, "  -p GSport  Specify the port number of the game server (optional).\n");
}
