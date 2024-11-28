#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include "common.h"

int is_number(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

int color_to_index(char color) {
    switch(color) {
        case 'R': return 0;
        case 'G': return 1;
        case 'B': return 2;
        case 'Y': return 3;
        case 'O': return 4;
        case 'P': return 5;
        default: return -1; // Invalid color
    }
}

void convert_code(char *temp, char *secret, int mode) {
    switch(mode) {
        case SECRET_TO_CODE:
            for (int i = 0; i < MAX_COLORS; i++) {
                temp[2*i] = secret[i];
                temp[2*i+1] = ' ';
            }
            temp[2*MAX_COLORS-1] = '\0';
            break;
        case CODE_TO_SECRET:
            for (int i = 0; i < MAX_COLORS; i++) {
                secret[i] = temp[2*i];
            }
            secret[MAX_COLORS] = '\0';
    }
}

