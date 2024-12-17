#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void handle_scoreboard_request(int tcp_socket) {
    printf("[DEBUG] Received SCOREBOARD request\n");
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    FindTopScores(buffer);
    send_tcp_response(buffer, tcp_socket);
}

int FindTopScores(char* buffer) {
    struct dirent **filelist;
    char filename[11];
    snprintf(filename, sizeof(filename), "scores.txt");
    int nentries, ifile;
    char fname[300];
    FILE *fp;
    char mode[8];
    char plid[ID_SIZE + 1];
    char score[4];
    char code[MAX_COLORS + 1];
    char notries[2];

    char buffer1[128], fileinformation[4096], *offset;
    offset = fileinformation;

    pthread_rwlock_rdlock(&scoreboard_lock);

    // Lê os arquivos do diretório SCORES/ e ordena alfabeticamente
    nentries = scandir("SCORES/", &filelist, 0, alphasort);
    if (nentries <= 2) { //TODO era isto certo branquinho?
        sprintf(buffer, "RSS EMPTY\n");
        pthread_rwlock_unlock(&scoreboard_lock);
        return 0;
    } else {
        ifile = 0;
        while (nentries--) {
            // Ignora arquivos ocultos e limita a 10 arquivos
            if (filelist[nentries]->d_name[0] != '.' && ifile < 10) {
                sprintf(fname, "SCORES/%s", filelist[nentries]->d_name);
                fp = fopen(fname, "r");
                if (fp != NULL) {
                    fscanf(fp, "%s %s %s %s %s", score, plid, code, notries, mode);

                    // Adiciona o texto no temp_buffer a partir da posição atual
                    sprintf(buffer1, "%03d %s %s %s %s\n", atoi(score), plid, code, notries, mode);
                    strcpy(offset, buffer1);
                    offset += strlen(buffer1);

                    fclose(fp);
                    ++ifile;
                }
            }
            free(filelist[nentries]);
        }
        free(filelist);
    }

    pthread_rwlock_unlock(&scoreboard_lock);

    int filesize = strlen(fileinformation);

    /* TODO em principio isto já não é necessário
    if (filesize == 0) {
        snprintf(buffer, 11, "RSS EMPTY\n");
        return 0;
    }
    */

    sprintf(buffer, "RSS OK %s %d %s\n", filename, filesize, fileinformation);


    return 0;
}
