#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_trials_request(int tcp_socket, char *request) {

    char buffer[GLOBAL_BUFFER];
    char plid[ID_SIZE + 1];
    memset(buffer, 0, sizeof(buffer));
    memset(plid, 0, sizeof(plid));

    int n = read_tcp_socket(tcp_socket, request, TRIALS_MSG_SIZE);
    if (n < 0 || strlen(request) > TRIALS_MSG_SIZE) {
        send_tcp_response("ERR\n", tcp_socket);
        return ERROR;
    }

    if (is_number(plid) == FALSE || sscanf(request, "%6s", plid) != 1) {
        send_tcp_response("ERR\n", tcp_socket);
        return ERROR;
    }

    Player *player = find_player(plid);

    if (player == NULL) {
        if (find_last_game(plid, buffer)) {
            send_tcp_response(buffer, tcp_socket);
        }
        else {
            send_tcp_response("RST NOK\n", tcp_socket);
        }
        set_verbose_trials_message(request, plid);
        return SUCCESS;
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_tcp_response("RST ERR\n", tcp_socket);
        return ERROR;
    }

    char filename[128];
    memset(filename, 0, sizeof(filename));

    snprintf(filename, sizeof(filename), "GAMES/GAME_%s.txt", player->plid);
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        if (settings.verbose_mode) {
            perror("Failed to open file");
        }
        send_tcp_response("RST ERR\n", tcp_socket);
        mutex_unlock(plid_mutex);
        return ERROR;
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
        if (settings.verbose_mode) {
            perror("Failed to read from file");
        }
        send_tcp_response("RST ERR\n", tcp_socket);
        fclose(file);
        mutex_unlock(plid_mutex);
        return ERROR;
    }
    char mode, code[5];
    if (sscanf(trash, "%d %c %s %d\n", &plid_num, &mode, code, &final_time) != 4) {
        send_tcp_response("RST ERR\n", tcp_socket);
        fclose(file);
        mutex_unlock(plid_mutex);
        return ERROR;
    }
    char *last_num = strrchr(trash, ' ');
    if (last_num == NULL) {
        send_tcp_response("RST ERR\n", tcp_socket);
        fclose(file);
        mutex_unlock(plid_mutex);
        return ERROR;
    }
    last_trial_num = atoi(last_num + 1);
    if (last_trial_num + final_time - current_time < 0) {
        player->current_game->end_status = 'T';
        time(&player->current_game->last_time);
        end_game(player, plid_mutex);
        if (find_last_game(plid, buffer)) {
            send_tcp_response(buffer, tcp_socket);
        }
        else {
            send_tcp_response("RST NOK\n", tcp_socket);
        }
        set_verbose_trials_message(request, plid);
        fclose(file);
        return SUCCESS;
    }
    while (fgets(trash, 1024, file) != NULL) {
        char c1, c2, c3, c4;
        int nb, nw, time;
        if (sscanf(trash, "T: %c%c%c%c %d %d %d\n", &c1, &c2, &c3, &c4, &nb, &nw, &time) != 7) {
            continue;
        }
        temp_ptr += sprintf(temp_ptr, "%c %c %c %c %d %d\n", c1, c2, c3, c4, nb, nw);
    }
    temp_ptr += sprintf(temp_ptr, "%ld", last_trial_num + final_time - current_time);
    filesize = strlen(temp) + 1;
    sprintf(pointer, "RST ACT %s %d %s\n\n", tempfilename, filesize, temp);

    send_tcp_response(buffer, tcp_socket);
    fclose(file);
    mutex_unlock(plid_mutex);
    set_verbose_trials_message(request, plid);
    return SUCCESS;
}

int find_last_game(char *PLID, char *buffer) {
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
                if (settings.verbose_mode) {
                    perror("File name too long");
                }
                free(filelist[i]);
                continue;
            }
        }

        free(filelist[i]);
    }
    free(filelist);

    FILE *file = fopen(latest_file, "r");
    if (!file) {
        if (settings.verbose_mode) {
            perror("Failed to open file");
        }
        return 0;
    }

    memset(buffer, 0, SMALL_BUFFER);
    int filesize = 0;
    fseek(file, 0, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    char filename[17];
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "STATE_%s.txt", PLID);
    snprintf(buffer, SMALL_BUFFER, "RST FIN %s %d ", filename, filesize+1);

    while (fgets(buffer + strlen(buffer), filesize + 1, file) != NULL) {
        continue;
    }
    strncat(buffer, "\n\n", SMALL_BUFFER);
    fclose(file);

    return 1;
}

void set_verbose_trials_message(char *request, const char *plid) {
    memset(request, 0, strlen(request));
    sprintf(request, "Trials request: PLID = %s\n", plid);
}