#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

void delete_directory_contents(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[GLOBAL_BUFFER];
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
    void clean_server();
    int tcp_socket = settings.tcp_socket;
    int udp_socket = settings.udp_socket;
    close(tcp_socket);
    close(udp_socket);
}

void sig_detected(int sig) {
    printf("Shutting down the server...\n");
    kill_sig(sig);
    cleanup_server();
    delete_directory_contents("GAMES");
    delete_directory_contents("SCORES");
    free(threads);
    printf("Deleted all files and directories.\n");
    exit(0);
}

void print_debug(int value) {
    printf("DEBUG: %d\n", value);
}