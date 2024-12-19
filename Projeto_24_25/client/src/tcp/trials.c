#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_show_trials(const char *input) {
    if (strcmp(input, "st") != 0 && strcmp(input, "show_trials") != 0) {
        return invalid_command_format(CMD_SHOW_TRIALS);
    }

    if(strcmp(plidCurr, "") == 0) {
        return errorCurrentPlayer(plidCurr);
    }
    
    int sockfd = connect_to_server(&res);
    if (sockfd < 0) return TRUE;

    send_show_trials_msg(sockfd);

    close(sockfd);
    freeaddrinfo(res);
    return TRUE;
}

void send_show_trials_msg(int fd) {
    char message[SMALL_BUFFER];
    snprintf(message, sizeof(message), "STR %s\n", plidCurr);

    if (send_tcp_message(fd, message) == ERROR) {
        fprintf(stderr, "ERROR: Failed to send 'show_trials' message\n");
    }

    receive_show_trials_msg(fd);
}

void receive_show_trials_msg(int fd) {
    char response[GLOBAL_BUFFER];
    char status[STATUS -2];
    char command[COMMAND];

    char filename[FILENAME_SIZE];
    int file_size;
    char *file_data = NULL;

    if (read_tcp_socket(fd, response, sizeof(response)) == ERROR) {
        printf("Response: %s\n", response);
        printf("ERROR: Failed to receive 'show_trials' response\n");
        return;
    }

    if (sscanf(response, "%s %s", command, status) < 2) {
        printf("ERROR: Failed to parse 'show_trials' response\n");
        return;
    }
    if (strcmp(command, "RST") != 0) {
        printf("ERROR: Unexpected 'show_trials' response\n");
        return;
    }

    if (strcmp(status, "ACT") == 0 || strcmp(status, "FIN") == 0) {
        if (strcmp(status, "FIN") == 0) {
            currPlayer = 0;
        }
        if (sscanf(response + strlen(command) + strlen(status) + 2, "%s %d\n", filename, &file_size) >= 1) {
            
            if(file_size > FSIZE) {
                printf("ERROR: File size too big\n");
                return;
            }

            if(strlen(filename) > 24) {
                filename[24] = '\0';
            }

            printf("Filename: %s, File size: %d\n", filename, file_size);

            char *file_data_start = strpbrk(response, "\n ");
            if (file_data_start != NULL) {
                file_data_start++;

                file_data = malloc(file_size + 1);
                if (!file_data) {
                    printf("ERROR: Memory allocation failed\n");
                    return;
                }

                strncpy(file_data, file_data_start, file_size);
                file_data[file_size] = '\0';

                FILE *fp = fopen(filename, "w");
                if (!fp) {
                    perror("Error saving file");
                    free(file_data);
                    return;
                }

                printf("%s\n", file_data_start);

                fwrite(file_data, 1, file_size, fp);
                fclose(fp);
                free(file_data);
            }
            else {
                printf("ERROR: Incomplete file data\n");
            }
        }
        else {
            printf("Unexpected server response: %s\n", response);
        }
    }
    else if (strcmp(status, "NOK") == 0) {
        printf("No game data available for player '%s'.\n", plidCurr);
    }
    else {
        printf("Unexpected server response: %s\n", response);
    }
}