#include "../include/constants.h"
#include "../include/prototypes.h"
#include "../include/globals.h"

char*GSIP = DEFAULT_IP;
int GSport = DEFAULT_PORT;
int currPlayer = 0;
int currTries = 0;
char plidCurr[ID_SIZE + 1];
int setPLID = 0;
int hasStarted = 0;
struct addrinfo *res;

int main(int argc, char *argv[]) {

    get_arguments(argc, argv);

    signal(SIGINT, sig_detected);

    int status;

    while (1) {
        char input[MAX_COMMAND_SIZE];
        memset(input, 0, sizeof(input));
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            fprintf(stderr, "Error reading command.\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        Command cmd = get_next_command(input);

        status = handle_command(cmd, input);
        if (status == EXIT_COMMAND) {
            return 0;
        }
    }
    return 0;
}