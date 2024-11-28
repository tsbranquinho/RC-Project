#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

// Função recursiva para apagar o conteúdo de um diretório
void delete_directory_contents(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignorar os diretórios "." e ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                delete_directory_contents(full_path);
                if (rmdir(full_path) != 0) {
                    perror("Error deleting directory");
                }
            } else {
                if (remove(full_path) != 0) {
                    perror("Error deleting file");
                }
            }
        } else {
            perror("Error getting file information");
        }
    }

    closedir(dir);
}

void sig_detected(int sig) {
    printf("Shutting down the server...\n");
    // TODO: free memory, close all sockets

    delete_directory_contents("GAMES");

    delete_directory_contents("SCORES");

    printf("Deleted all files and directories.\n");
    exit(0);
}
