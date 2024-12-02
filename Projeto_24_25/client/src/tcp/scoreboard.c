#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_show_scoreboard(const char *input) {
    if (strcmp(input, "sb") != 0 && strcmp(input, "scoreboard") != 0) {
        return invalid_command_format(CMD_SCOREBOARD);
    }
    int sockfd = connect_to_server(&res);
    if (sockfd < 0) return TRUE;

    send_show_scoreboard_msg(sockfd);

    close(sockfd);
    freeaddrinfo(res);
    return TRUE;
}

void send_show_scoreboard_msg(int fd) {
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "SSB\n");

    if (send_tcp_message(fd, message) == -1) {
        fprintf(stderr, "ERROR: Failed to send 'show_scoreboard' message\n");
    }

    receive_show_scoreboard_msg(fd);
}

void receive_show_scoreboard_msg(int fd) {
    char response[BUFFER_SIZE * 10];
    if (read_tcp_socket(fd, response, sizeof(response)) == -1) {
        fprintf(stderr, "ERROR: Failed to receive 'show_scoreboard' response\n");
        return;
    }

    char status[10], filename[1024]; //TODO
    int file_size;
    char *file_data = NULL;

    if (sscanf(response, "RSS %s %s %d\n", status, filename, &file_size) >= 1) {
        if (strcmp(status, "OK") == 0) {
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

            printf("Scoreboard saved to '%s'.\n", filename);
            printf("Top 10 Scores:\n%s\n", file_data);
        } else if (strcmp(status, "EMPTY") == 0) {
            printf("The scoreboard is empty.\n");
        } else {
            printf("Unexpected server response: %s\n", response);
        }
    } else {
        printf("Invalid response format.\n");
    }
}