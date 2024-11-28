#ifndef __COMMON_H__
#define __COMMON_H__

#define GROUP_NUMBER 0                          // FIXME: Replace with your group number
#define DEFAULT_PORT (58000 + GROUP_NUMBER)     // FIXME - Default GROUP NUMBER
#define DEFAULT_IP "127.0.0.1"      // Default IP if not specified -> localhost
#define ID_SIZE 6                   // Size of player ID
#define MAX_PLAYTIME 600            // Maximum playtime in seconds
#define MAX_COLORS 4                // Maximum number of colors in the colour key
#define MAX_TRIALS 8                // Maximum number of trials
#define EXIT_FAILURE 1              // Exit failure
#define COLOR_OPTIONS "RGBYOP"      // Possible colors
#define SECRET_TO_CODE 0
#define CODE_TO_SECRET 1
#define FALSE 0
#define TRUE 1
#define EXIT_COMMAND 2
#define EMPTY " "

int is_number(const char *str);
int color_to_index(char color);
void convert_code(char *temp, char *secret, int mode);

#endif
