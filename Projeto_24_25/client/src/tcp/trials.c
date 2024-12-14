#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_show_trials(const char *input) {
    //TODO verificar PLID
    if (strcmp(input, "st") != 0 && strcmp(input, "show_trials") != 0) {
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
    snprintf(message, sizeof(message), "STR %s\n", plidCurr);

    if (send_tcp_message(fd, message) == -1) {
        fprintf(stderr, "ERROR: Failed to send 'show_trials' message\n");
    }

    receive_show_trials_msg(fd);
}

void receive_show_trials_msg(int fd) {
    char response[4096];
    char status[4];
    char command[4];

    char filename[1024]; //TODO not sure o tamanho que ponho
    int file_size;
    char *file_data = NULL;

    if (read_tcp_socket(fd, response, sizeof(response)) == -1) {
        printf("ERROR: Failed to receive 'show_trials' response\n");
        return;
    }

    if (sscanf(response, "%s %s ", command, status) != 1) {
        printf("ERROR: Failed to parse 'show_trials' response\n");
        return;
    }
    if (strcmp(command, "RST") != 0) {
        printf("ERROR: Unexpected 'show_trials' response\n");
        return;
    }

    if (strcmp(status, "ACT") == 0 || strcmp(status, "FIN") == 0) {
        if (sscanf(response + strlen(command) + strlen(status) + 2, "%s %d\n", filename, &file_size) >= 1) {
            printf("Filename: %s, File size: %d\n", filename, file_size);

            // A parte após o '\n' no response contém os dados do arquivo
            char *file_data_start = strchr(response, '\n');  // Procurar o primeiro '\n'
            if (file_data_start != NULL) {
                file_data_start++; // Mover para o próximo caractere após o '\n'

                // Alocar memória para armazenar os dados do arquivo
                file_data = malloc(file_size + 1); // +1 para o terminador nulo
                if (!file_data) {
                    printf("ERROR: Memory allocation failed\n");
                    return;
                }

                // Copiar os dados do arquivo para o buffer
                strncpy(file_data, file_data_start, file_size);
                file_data[file_size] = '\0'; // Garantir que os dados do arquivo sejam uma string válida

                // Agora podemos salvar o arquivo
                FILE *fp = fopen(filename, "w");
                if (!fp) {
                    perror("Error saving file");
                    free(file_data); // Liberar a memória
                    return;
                }

                // Salvar os dados do arquivo
                fwrite(file_data, 1, file_size, fp);
                fclose(fp);

                printf("Trials saved to '%s'.\n", filename);
                printf("Game Summary:\n%s\n", file_data);

                // Liberar a memória alocada para os dados do arquivo
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