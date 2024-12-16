#include "../../include/constants.h"
#include "../../include/prototypes.h"
#include "../../include/globals.h"

pthread_mutex_t *get_plid_mutex(const char *plid) {
    unsigned int index = hash_lock(plid); // Now used for indexing

    pthread_mutex_lock(&lock_table_mutex); // Protect access to lock_table
    if (lock_table_plid[index] == NULL) {
        lock_table_plid[index] = malloc(sizeof(pthread_mutex_t));
        if (!lock_table_plid[index]) {
            perror("Failed to allocate memory for PLID mutex");
            pthread_mutex_unlock(&lock_table_mutex);
            return NULL;
        }
        pthread_mutex_init(lock_table_plid[index], NULL);
    }
    pthread_mutex_unlock(&lock_table_mutex); // Release lock_table access

    return lock_table_plid[index]; // Return the lock for this PLID
}

void cleanup_plid_mutex(const char *plid) {
    unsigned int index = hash_lock(plid);

    pthread_mutex_lock(&lock_table_mutex);
    if (lock_table_plid[index] != NULL) {
        pthread_mutex_destroy(lock_table_plid[index]);
        free(lock_table_plid[index]);
        lock_table_plid[index] = NULL;
    }
    pthread_mutex_unlock(&lock_table_mutex);
}

Player *find_player(const char *plid) {
    printf("Finding player with ID %s\n", plid);
    unsigned int index = hash(plid);

    pthread_rwlock_rdlock(&hash_table_lock);
    Player *current = hash_table[index];

    while (current != NULL) {
        if (strcmp(current->plid, plid) == 0) {
            printf("Player with ID %s found\n", plid);
            pthread_rwlock_unlock(&hash_table_lock);
            return current;
        }
        current = current->next;
    }

    pthread_rwlock_unlock(&hash_table_lock);
    printf("Player with ID %s not found\n", plid);
    return NULL;
}

void remove_player(const char *plid, pthread_mutex_t *plid_mutex) {
    unsigned int index = hash(plid);

    pthread_rwlock_wrlock(&hash_table_lock);
    Player *current = hash_table[index];
    Player *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->plid, plid) == 0) {
            if (prev == NULL) {
                hash_table[index] = current->next;
            } else {
                prev->next = current->next;
            }

            free(current->current_game);
            free(current);

            pthread_rwlock_unlock(&hash_table_lock);
            mutex_unlock(plid_mutex); //TODO Mutex
            cleanup_plid_mutex(plid);
            return;
        }
        prev = current;
        current = current->next;
    }

    pthread_rwlock_unlock(&hash_table_lock);
    printf("Player with ID %s not found\n", plid);
}

Player *create_player(const char *plid) {
    Player *player = malloc(sizeof(Player));
    if (!player) {
        perror("Failed to allocate memory for Player");
        return NULL;
    }

    strncpy(player->plid, plid, ID_SIZE);
    player->plid[ID_SIZE] = '\0';
    player->current_game = NULL;
    player->next = NULL;

    pthread_mutex_t *plid_mutex = get_plid_mutex(plid);
    if (!plid_mutex) {
        free(player);
        return NULL; // Ensure the mutex is properly handled
    }

    return player;
}

void insert_player(Player *player) {
    unsigned int index = hash(player->plid);

    pthread_rwlock_wrlock(&hash_table_lock);
    player->next = hash_table[index];
    hash_table[index] = player;
    pthread_rwlock_unlock(&hash_table_lock);
}

unsigned int hash(const char *plid) {
    unsigned int hash_value = 0;
    while (*plid) {
        hash_value = (hash_value << 5) + *plid++;
    }
    return hash_value % MAX_PLAYERS;
}

unsigned int hash_lock(const char *plid) {
    unsigned int hash_value = 0;
    while (*plid) {
        hash_value = (hash_value << 5) + *plid++;
    }
    return hash_value % MAX_LOCKS;
}

