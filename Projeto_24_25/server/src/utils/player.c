#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

Player *find_player(const char *plid) {
    unsigned int index = hash(plid);
    Player *current = hash_table[index];

    while (current != NULL) {
        if (strcmp(current->plid, plid) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void remove_player(const char *plid) {
    unsigned int index = hash(plid);
    Player *current = hash_table[index];
    Player *prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->plid, plid) == 0) {
            if (prev == NULL) {
                hash_table[index] = current->next;  // Remove from the head of the list
            } else {
                prev->next = current->next;  // Remove from the middle or end of the list
            }
            free(current->current_game);  // Free the associated game
            free(current);  // Free the player
            return;
        }
        prev = current;
        current = current->next;
    }
    printf("Player with ID %s not found\n", plid);  // Player not found
}

Player *create_player(const char *plid) {
    Player *player = malloc(sizeof(Player));
    strncpy(player->plid, plid, ID_SIZE);
    player->plid[ID_SIZE] = '\0';
    player->is_playing = 0;
    player->current_game = NULL;
    player->next = NULL;  // No next player yet
    return player;
}

unsigned int hash(const char *plid) {
    unsigned int hash_value = 0;
    while (*plid) {
        hash_value = (hash_value << 5) + *plid++;
    }
    return hash_value % MAX_PLAYERS;
}

void insert_player(Player *player) {
    unsigned int index = hash(player->plid);
    player->next = hash_table[index];
    hash_table[index] = player;
}