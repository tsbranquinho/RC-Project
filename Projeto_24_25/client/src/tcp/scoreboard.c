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
    char message[SMALL_BUFFER];
    snprintf(message, sizeof(message), "SSB\n");

    if (send_tcp_message(fd, message) == ERROR) {
        fprintf(stderr, "ERROR: Failed to send 'show_scoreboard' message\n");
    }

    receive_show_scoreboard_msg(fd);
}

void receive_show_scoreboard_msg(int fd) {
    char response[GLOBAL_BUFFER];
    char status[STATUS];
    char command[COMMAND];

    char filename[FILENAME_SIZE];
    int file_size;
    char *file_data = NULL;

    if (read_tcp_socket(fd, response, sizeof(response)) == ERROR) {
        printf("ERROR: Failed to receive 'show_scoreboard' response\n");
        return;
    }

    if (sscanf(response, "%s %s", command, status) < 2) {
        printf("ERROR: Failed to parse 'show_scoreboard' response\n");
        return;
    }
    
    if (strcmp(command, "RSS") != SUCCESS) {
        printf("ERROR: Unexpected 'show_scoreboard' response\n");
        return;
    }

    if (strcmp(status, "OK") == 0) {
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

                fwrite(file_data, 1, file_size, fp);
                fclose(fp);

                
                printf("%s\n", file_data_start);
                
                free(file_data);
            }
            else {
                printf("ERROR: Incomplete file data\n");
            }
        }
        else {
            printf("Unexpected server response1: %s\n", response);
        }
    }
    else if (strcmp(status, "EMPTY") == 0) {
        printf("Scoreboard empty.\n");
    }
    else {
        printf("Unexpected server response2: %s\n", response);
    }
}