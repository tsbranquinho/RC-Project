#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

void delete_files_in_directory(const char *dir_path) {
    struct dirent *entry;
    DIR *dir = opendir(dir_path);

    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char filepath[1024];
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);

        struct stat path_stat;
        if (stat(filepath, &path_stat) == -1) {
            perror("Error getting file info");
            continue;
        }

        // Verifica se é um arquivo regular
        if (S_ISREG(path_stat.st_mode)) {
            if (remove(filepath) != 0) {
                fprintf(stderr, "Error deleting file: %s\n", filepath);
            }
        }
    }

    closedir(dir);
}

void sig_detected(int sig) {
    printf("Shutting down the server...\n");
    // TODO: free memory, close all sockets

    // Deletar apenas arquivos das pastas
    delete_files_in_directory("GAMES");
    delete_files_in_directory("SCORES");

    printf("All files deleted.\n");
    exit(0);
}