#include "../include/constants.h"
#include "../include/prototypes.h"
#include "../include/globals.h"

char*GSIP = DEFAULT_IP;                  // Default IP
int GSport = DEFAULT_PORT;               // Default port
int currPlayer = 0;                      // Flag to check if a player is playing or not
int currTries = 0;                       // Number of tries of the current player (it starts with the "1st try")
char plidCurr[ID_SIZE + 1];              // Current player ID
int setPLID = 0;
struct addrinfo *res;

int main(int argc, char *argv[]) {

    get_arguments(argc, argv);

    signal(SIGINT, sig_detected);

    int status;

    while (1) {
        printf("Enter command: ");
        char input[MAX_COMMAND_SIZE];
        memset(input, 0, sizeof(input));
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            fprintf(stderr, "Error reading command.\n");
            continue;
        }

        input[strcspn(input, "\n")] = '\0';

        Command cmd = get_next_command(input);

        status = handle_command(cmd, input); //don't know if we need this but for now let's go
        if (status == EXIT_COMMAND) {
            return 0;
        }
    }
    return 0;
}