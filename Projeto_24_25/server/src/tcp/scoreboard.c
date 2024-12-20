#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_scoreboard_request(int tcp_socket, char *request) {
    int n = read_tcp_socket(tcp_socket, request, 1);
    if (n != 0) {
        send_tcp_response("ERR\n", tcp_socket);
        return ERROR;
    }
    char buffer[2*GLOBAL_BUFFER];
    memset(buffer, 0, sizeof(buffer));
    find_top_scores(buffer);
    memset(request, 0, SMALL_BUFFER);
    strcpy(request, "Scoreboard request\n");
    send_tcp_response(buffer, tcp_socket);
    return SUCCESS;
}

void find_top_scores(char* buffer) {
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

    char buffer1[SMALL_BUFFER], fileinformation[2*GLOBAL_BUFFER], *offset;
    offset = fileinformation;

    pthread_rwlock_rdlock(&scoreboard_lock);

    // Read the directory to find all entries
    nentries = scandir("SCORES/", &filelist, 0, alphasort);
    if (nentries <= 2) { 
        sprintf(buffer, "RSS EMPTY\n");
        pthread_rwlock_unlock(&scoreboard_lock);
        return;
    } else {
        ifile = 0;
        while (nentries--) {
            // Ignores hidden files and limits the number of found files to 10
            if (filelist[nentries]->d_name[0] != '.' && ifile < 10) {
                sprintf(fname, "SCORES/%s", filelist[nentries]->d_name);
                fp = fopen(fname, "r");
                if (fp != NULL) {
                    fscanf(fp, "%s %s %s %s %s", score, plid, code, notries, mode);

                    // Adds the information to the buffer
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

    sprintf(buffer, "RSS OK %s %d %s\n", filename, filesize, fileinformation);

    return;
}
