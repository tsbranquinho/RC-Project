#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_trials_request(Player *player) {
    //TODO change the arguments

    if (player == NULL) {
        //TODO send NOK
        printf("Player not found\n");
        return;
    }
    if (!player->is_playing) {
        //TODO send FIN e enviar os jogos mais recentes do player (função de ordenação no guia e no fim)
        printf("Player is not playing\n");
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