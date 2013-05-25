
#ifndef _parse_h_
#define _parse_h_

#define BUF_MAX  (81)
#define NAME_MAX (21)

/* Struct for pcb, has name of process, total lifetime, running time in running
   state, and the time in current state */
typedef struct
{
    char name[NAME_MAX];
    int  lifetime;
    int  runningTime;
    int  time_in_state;
} pcb_t;

/* State functions */
void readyState();
void runningState();
void blockedState();
void stateTransitions(int the_signal);

/* Intterupt Handlers and Functions*/

void moveToBlockedState(int signal);
void cleanupAndExit(int signal);
int setUpStateLister();
int setUpBlocker();
int setUpConfigUpdater();
int setUpExit();

/* Queue display */
void printQueue(List_t *list, char *name);
void displayQueueInfo(int signal);
void updateQueueTime(List_t *list);

/* Config functions */
int readConfigTimer();
int setUpAlarm(int timer);
void updateConfigTimer(int signal);

#endif /* _parse_h_ */

