#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

int handle_clean(const char* input) {
    if (strlen(input) != 5) {
        return invalid_command_format(CMD_CLEAN);
    }

    DIR* dir;
    struct dirent* entry;
    int deleted_count = 0;

    // Open the current directory
    dir = opendir(".");
    if (dir == NULL) {
        perror("Failed to open directory");
        return TRUE;
    }

    // Iterate through each entry in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Check if the file ends with ".txt"
        char* ext = strrchr(entry->d_name, '.');
        if (ext != NULL && strcmp(ext, ".txt") == 0) {
            // Attempt to remove the file
            if (remove(entry->d_name) == 0) {
                deleted_count++;
            } else {
                perror("Failed to delete file");
            }
        }
    }

    closedir(dir);

    printf("Total .txt files deleted: %d\n", deleted_count);
    return TRUE;
}
