#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_trials_request(int tcp_socket) {
    //TODO mandar ficheiro alterado
    //TODO close if time less than 0

    printf("[DEBUG] Received TRIALS request\n");
    char buffer[GLOBAL_BUFFER];
    char plid[ID_SIZE + 1];
    memset(buffer, 0, sizeof(buffer));
    memset(plid, 0, sizeof(plid));

    int n = read_tcp_socket(tcp_socket, buffer, 8);
    buffer[n-1] = '\0';
    printf("[DEBUG] Received message: %s\n", buffer);

    if (is_number(plid) == FALSE || sscanf(buffer, "%6s", plid) != 1) {
        if(settings.verbose_mode) {
            printf("Invalid TRIALS request\n");
        }
        send_tcp_response("ERR\n", tcp_socket);
        return;
    }

    if(settings.verbose_mode) {
        printf("TRIALS request from %s\n", plid);
    }
    
    plid[ID_SIZE] = '\0';
    Player *player = find_player(plid);

    if (player == NULL) {
        if (FindLastGame(plid, buffer)) {
            send_tcp_response(buffer, tcp_socket);
        }
        else {
            send_tcp_response("RST NOK\n", tcp_socket);
        }
        return;
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_tcp_response("RST ERR\n", tcp_socket);
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

    char tempfilename[128];
    sprintf(tempfilename, "STATE_%s.txt", player->plid);
    memset(buffer, 0, sizeof(buffer));
    int filesize, last_trial_num, final_time, plid_num;
    char *pointer = buffer;
    char temp[2048];
    char *temp_ptr = temp;

    time_t current_time;
    time(&current_time);
    char trash[1024];
    if (fgets(trash, 1024, file) == NULL) {
        return;
    }
    char mode, code[5];
    if (sscanf(trash, "%d %c %s %d\n", &plid_num, &mode, code, &final_time) != 4) {
        send_tcp_response("RST ERR\n", tcp_socket);
        fclose(file);
        return;
    }
    char *last_num = strrchr(trash, ' ');
    if (last_num == NULL) {
        perror("strrchr");
        send_tcp_response("RST ERR\n", tcp_socket);
        fclose(file);
        return;
    }
    last_trial_num = atoi(last_num + 1);
    while (fgets(trash, 1024, file) != NULL) {
        char c1, c2, c3, c4;
        int nb, nw, time;
        printf("Trash: %s\n", trash);
        if (sscanf(trash, "T: %c%c%c%c %d %d %d\n", &c1, &c2, &c3, &c4, &nb, &nw, &time) != 7) {
            continue;
        }
        printf("C1: %c, C2: %c, C3: %c, C4: %c, NB: %d, NW: %d, TIME: %d\n", c1, c2, c3, c4, nb, nw, time);
        temp_ptr += sprintf(temp_ptr, "%c %c %c %c %d %d\n", c1, c2, c3, c4, nb, nw);
    }
    temp_ptr += sprintf(temp_ptr, "%ld", last_trial_num + final_time - current_time);
    filesize = strlen(temp) + 1;
    sprintf(pointer, "RST ACT %s %d %s\n\n", tempfilename, filesize, temp);

    send_tcp_response(buffer, tcp_socket);
    fclose(file);

    mutex_unlock(plid_mutex);
}

int FindLastGame(char *PLID, char *buffer) {
    struct dirent **filelist;
    int n_entries;
    char dirname[SMALL_BUFFER], latest_file[FSIZE];
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

    memset(buffer, 0, SMALL_BUFFER);
    int filesize = 0;
    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    snprintf(buffer, SMALL_BUFFER, "RST FIN %s %d ", strrchr(latest_file, '/') + 1, filesize+1);

    while (fgets(buffer + strlen(buffer), filesize + 1, file) != NULL) {
        continue;
    }
    strncat(buffer, "\n", SMALL_BUFFER);
    fclose(file);

    return 1;
}