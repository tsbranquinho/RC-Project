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
    char status[STATUS-2];
    char command[COMMAND];
    char temp[5];

    char filename[FILENAME_SIZE];
    int file_size = 0;
    char *file_data = NULL;
    char *response_ptr = response;

    if (read_tcp_socket(fd, response, sizeof(response)) == ERROR) {
        printf("Response: %s\n", response);
        printf("ERROR: Failed to receive 'show_trials' response\n");
        return;
    }
    response_ptr += get_word(command, response_ptr);
    if (strcmp(command, "RST") != 0) {
        printf("ERROR: Invalid response command\n");
        return;
    }
    response_ptr += get_word(status, response_ptr);
    if (strcmp(status, "NOK") == 0) {
        printf("No game found for this player.\n");
        return;
    }
    if (strcmp(status, "ERR") == 0) {
        printf("Error getting trials. Please try again later.\n");
        return;
    }
    if (strcmp(status, "ACT") == 0 || strcmp(status, "FIN") == 0) {
        if (strcmp(status, "FIN") == 0) {
            currPlayer = 0;
        }
        response_ptr += get_word(filename, response_ptr);
        response_ptr += get_word(temp, response_ptr);
        file_size = atoi(temp);
        if (file_size < 0) {
            printf("ERROR: Invalid file size\n");
            return;
        }
        if (*response_ptr == '\n') {
            response_ptr++;
        }
        file_data = response_ptr;
        char *file_data_ptr = file_data;
        FILE *file = fopen(filename, "w");
        if (!file) {
            perror("Error opening file");
            return;
        }
        if (fwrite(file_data_ptr, 1, file_size, file) != file_size) {
            perror("Error writing to file");
            fclose(file);
            return;
        }
        fclose(file);
        printf("Trials saved to file %s\n", filename);
    } else {
        printf("ERROR: Invalid response status\n");
        return;
    }
}


int get_word(char *word, char *response) {
    int i = 0;
    while (*response != ' ' && *response != '\n') {
        word[i] = *response;
        response++;
        i++;
    }
    word[i] = '\0';
    return ++i;
}