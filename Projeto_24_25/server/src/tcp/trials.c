#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_trials_request(int tcp_socket) {
    //TODO mandar ficheiro alterado

    printf("[DEBUG] Received TRIALS request\n");
    char buffer[4096];
    char plid[ID_SIZE + 1];
    memset(buffer, 0, sizeof(buffer));
    memset(plid, 0, sizeof(plid));

    int n = read_tcp_socket(tcp_socket, buffer, 8);
    buffer[n-1] = '\0';
    printf("[DEBUG] Received message: %s\n", buffer);

    if (is_number(plid) == FALSE || sscanf(buffer, "%6s", plid) != 1) {
        //TODO send ERR
        send_tcp_response("ERR\n", tcp_socket);
        return;
    }

    plid[ID_SIZE] = '\0';
    printf("[DEBUG] Received TRIALS request from %s\n", plid);
    printf("plid: %s\n", plid);
    Player *player = find_player(plid);

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_tcp_response("RST ERR\n", tcp_socket);
        return;
    }

    if (player == NULL) {
        if (FindLastGame(plid, buffer)) {
            send_tcp_response(buffer, tcp_socket);
        }
        else {
            send_tcp_response("RST NOK\n", tcp_socket);
        }
        mutex_unlock(plid_mutex);
        return;
    }

    char filename[128];
    memset(filename, 0, sizeof(filename));

    snprintf(filename, sizeof(filename), "GAMES/GAME_%s.txt", player->plid);
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("fopen");
        send_tcp_response("RST ERR\n", tcp_socket); //TODO acho que é isto
        mutex_unlock(plid_mutex);
        return;
    }

    char* tempfilename = strrchr(filename, '/');
    tempfilename++;
    memset(buffer, 0, sizeof(buffer));
    int filesize = 0;
    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    snprintf(buffer, sizeof(buffer), "RST ACT %s %d\n", tempfilename, filesize);

    while (fgets(buffer + strlen(buffer), filesize + 1, file) != NULL) {
        continue;
    }
    printf("buffer: %s\n", buffer);
    send_tcp_response(buffer, tcp_socket);
    fclose(file);

    mutex_unlock(plid_mutex);
}

int FindLastGame(char *PLID, char *buffer) {
    struct dirent **filelist;
    int n_entries;
    char dirname[128], latest_file[512]; // TODO valores tirados do cu
    memset(dirname, 0, sizeof(dirname));
    memset(latest_file, 0, sizeof(latest_file));

    snprintf(dirname, sizeof(dirname), "GAMES/%s", PLID);

    n_entries = scandir(dirname, &filelist, NULL, alphasort);
    if (n_entries <= 2) {
        return 0;
    }

    for (int i = 0; i < n_entries; i++) {
        if (filelist[i]->d_name[0] == '.') {
            free(filelist[i]);
            continue;
        }

        if (strlen(filelist[i]->d_name) >= 15 && isdigit(filelist[i]->d_name[0])) {
            int written = snprintf(latest_file, sizeof(latest_file), "%s/%s", dirname, filelist[i]->d_name);
            if (written < 0 || written >= sizeof(latest_file)) {
                fprintf(stderr, "Error: File path too long!\n");
                free(filelist[i]);
                continue;
            }
        }

        free(filelist[i]);
    }
    free(filelist);

    FILE *file = fopen(latest_file, "r");
    if (!file) {
        perror("fopen");
        return 0;
    }


    //TODO valores todos errados
    memset(buffer, 0, BUFFER_SIZE);
    snprintf(buffer, BUFFER_SIZE, "RST FIN %s\n", strrchr(latest_file, '/') + 1);

    size_t header_length = strlen(buffer);
    size_t bytes_read = fread(buffer + header_length, 1, BUFFER_SIZE - header_length - 1, file);

    buffer[header_length + bytes_read] = '\0';
    fclose(file);

    return 1;
}