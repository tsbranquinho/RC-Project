#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "constants.h"

extern Player *hash_table[MAX_PLAYERS];

extern pthread_mutex_t lock_table_mutex;
extern pthread_mutex_t *lock_table_plid[MAX_LOCKS];
extern pthread_rwlock_t hash_table_lock;
extern pthread_rwlock_t scoreboard_lock;

extern set settings;
extern TaskQueue task_queue;
#endif