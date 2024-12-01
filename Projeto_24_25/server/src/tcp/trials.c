#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_trials_request(int tcp_socket) {
    char buffer[BUFFER_SIZE];
    char plid[ID_SIZE + 1];
    int n = read(tcp_socket, buffer, BUFFER_SIZE);
    if (n < 0) {
        perror("Failed to read from TCP socket");
        return;
    }
    buffer[n] = '\0';
    if (sscanf(buffer, " %6s", plid) != 1) {
        //TODO send ERR
        send_tcp_response("ERR\n", tcp_socket);
        return;
    }
    printf("[DEBUG] Received TRIALS request from %s\n", plid);
    Player *player = find_player(plid);
    if (player == NULL) {
        //TODO send NOK
        send_tcp_response("NOK\n", tcp_socket);
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
        return;
    }

    char filename[128];
    memset(filename, 0, sizeof(filename));
    snprintf(filename, sizeof(filename), "GAMES/GAME_%s.txt", player->plid);
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }
    send_tcp_response("OK\n", tcp_socket);
    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        send_tcp_response(buffer, tcp_socket);
    }
    fclose(file);
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