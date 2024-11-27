#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "constants.h"

//Global variables
extern char *GSIP;                // Default IP
extern int GSport;              // Default port
extern char plidCurr[ID_SIZE+1];              // Current player ID
extern int currPlayer;                      // Flag to check if a player is playing or not
extern int currTries;                       // Number of tries of the current player (it starts with the "1st try")

#endif