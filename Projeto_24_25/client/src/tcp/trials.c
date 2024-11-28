#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_show_trials(const char *input) {
    if (strcmp(input, "st") != 0 || strcmp(input, "show_trials") != 0) {
        return invalid_command_format(CMD_SHOW_TRIALS);
    }
    int sockfd = connect_to_server(&res);
    if (sockfd < 0) return TRUE;

    send_show_trials_msg(sockfd);

    close(sockfd);
    freeaddrinfo(res);
    return TRUE;
}

void send_show_trials_msg(int fd) {
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "STR %d\n", currPlayer);

    if (send_tcp_message(fd, message) == -1) {
        fprintf(stderr, "ERROR: Failed to send 'show_trials' message\n");
    }

    receive_show_trials_msg(fd);
}

void receive_show_trials_msg(int fd) {
    char response[BUFFER_SIZE * 10];
    if (read_tcp_socket(fd, response, sizeof(response)) == -1) {
        fprintf(stderr, "ERROR: Failed to receive 'show_trials' response\n");
        return;
    }

    char status[10], filename[1024]; //TODO not sure o tamanho que ponho
    int file_size;
    char *file_data = NULL;

    if (sscanf(response, "RST %s %s %d\n", status, filename, &file_size) >= 1) {
        if (strcmp(status, "ACT") == 0 || strcmp(status, "FIN") == 0) {
            file_data = strstr(response, "\n\n") + 2;
            if (!file_data || strlen(file_data) != (size_t)file_size) {
                fprintf(stderr, "Incomplete file data.\n");
                return;
            }

            FILE *fp = fopen(filename, "w");
            if (!fp) {
                perror("Error saving file");
                return;
            }
            fwrite(file_data, 1, file_size, fp);
            fclose(fp);

            printf("Trials saved to '%s'.\n", filename);
            printf("Game Summary:\n%s\n", file_data);
        } else if (strcmp(status, "NOK") == 0) {
            printf("No game data available for player '%d'.\n", currPlayer);
        } else {
            printf("Unexpected server response: %s\n", response);
        }
    } else {
        printf("Invalid response format.\n");
    }
}