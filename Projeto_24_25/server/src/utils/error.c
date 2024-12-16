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


void cleanup_server() {
    printf("Cleaning up server resources...\n");

    pthread_rwlock_wrlock(&hash_table_lock);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        Player *current = hash_table[i];
        while (current != NULL) {
            Player *next = current->next;

            if (current->current_game) {
                free(current->current_game);
            }

            free(current);
            current = next;
        }
        hash_table[i] = NULL;
    }

    pthread_rwlock_unlock(&hash_table_lock);

    pthread_mutex_lock(&lock_table_mutex);

    for (int i = 0; i < MAX_LOCKS; i++) {
        if (lock_table_plid[i] != NULL) {
            pthread_mutex_destroy(lock_table_plid[i]);
            free(lock_table_plid[i]);
            lock_table_plid[i] = NULL;
        }
    }

    pthread_mutex_unlock(&lock_table_mutex);

    printf("Server resources cleaned up successfully.\n");
}

void sig_detected(int sig) {
    printf("Shutting down the server...\n");

    // TODO: free memory, close all sockets, threads....
    //cleanup_server(); // Limpar Hash Table e Lock Table

    delete_directory_contents("GAMES");

    delete_directory_contents("SCORES");

    printf("Deleted all files and directories.\n");
    exit(0);
}

void print_debug(int value) {
    printf("DEBUG: %d\n", value);
}