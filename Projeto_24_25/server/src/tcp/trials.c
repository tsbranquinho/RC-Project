#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_trials_request(int tcp_socket) {

    printf("[DEBUG] Received TRIALS request\n");
    char buffer[4096];
    char plid[ID_SIZE + 1];
    memset(buffer, 0, sizeof(buffer));
    memset(plid, 0, sizeof(plid));

    int n = read_tcp_socket(tcp_socket, buffer, 8);
    buffer[n-1] = '\0';
    printf("[DEBUG] Received message: %s\n", buffer);

    if (sscanf(buffer, "%6s", plid) != 1) {
        //TODO send ERR
        send_tcp_response("ERR\n", tcp_socket);
        return;
    }

    plid[ID_SIZE] = '\0';
    printf("[DEBUG] Received TRIALS request from %s\n", plid);
    printf("plid: %s\n", plid);
    Player *player = find_player(plid);
    if (player == NULL) {
        //TODO send NOK
        printf("Player not found\n");
        send_tcp_response("NOK\n", tcp_socket);
        return;
    }

    pthread_mutex_t *plid_mutex = mutex_plid(plid);
    if (!plid_mutex) {
        send_tcp_response("ERR\n", tcp_socket);
        return;
    }

    if (!player->is_playing) {
        //TODO send FIN e enviar os jogos mais recentes do player (função de ordenação no guia e no fim)
        //I truly have no idea if this is what it's supposed to do
        if (FindLastGame(plid, buffer)) {
            send_tcp_response(buffer, tcp_socket);
        }
        else {
            send_tcp_response("FIN\n", tcp_socket);
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
        send_tcp_response("ERR\n", tcp_socket); //TODO acho que é isto
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
    snprintf(buffer, sizeof(buffer), "ACT RST %s %d\n", tempfilename, filesize);
    while (fgets(buffer + strlen(buffer), filesize + 1, file) != NULL) {
        continue;
    }
    printf("buffer: %s\n", buffer);
    send_tcp_response(buffer, tcp_socket);
    fclose(file);

    mutex_unlock(plid_mutex);
    //no fundo é enviar este ficheiro por TCP
}

int FindLastGame(char *PLID, char *filename) {
    struct dirent **filelist;
    int n_entries, found;
    char dirname[20];

    sprintf(dirname, "GAMES/%s", PLID);

    n_entries = scandir(dirname, &filelist, 0, alphasort);

    found = 0;

    if (n_entries < 0) {
        return 0;
    }
    else {
        while (n_entries--) {
            if (filelist[n_entries]->d_name[0] != '.' && !found) {
                sprintf(filename, "GAMES/%s/%s", PLID, filelist[n_entries]->d_name);
                found = 1;
            }
            free(filelist[n_entries]);
        }
        free(filelist);
    }
    return(found);
}