README
======

This is an assignment for my Operating Systems class CSCI 3120

It is an implementation of the 5 state process management model using
interrupts in C.

This program simulates processes going through the 5 state model for
process management using intterupt handlers.

Processes start in a new queue when they are created, and they travel too the
ready queue right away. They then go into the running queue if possible and
run for their running duration. Processes can be blocked by system intterupts
and will go into the exit queue when they have ran for their lifetime
duration.

Requirements
------------

This program requires `gcc` and `make` to compile.

This program is made to run on the bluenose.cs.dal.ca servers but should work
on most linux distros.

Installation
------------

1. Copy files into working directory.
2. run `make` command in terminal
3. run `./simulate`

Making the Program Do Things
----------------------------

1. Type in a proccess in the form of
   `processname total_duration running_state_duration`
   ex. firefox 20 3

   This would make a proccess called firefox run for 3 units of time whenever it
   is in the running queue, and exit once it has ran for 20 units of time total.

2. Send intterupts to the program using kill -intteruptname proccessID
   Where proccessID is the number of the `simulate` proccess and the intterupt
   name is one below.

   SIGUSR1 - print the state of the system (list the contents of the ready,
   running, and blocked, queue)

   SIGUSR2 - the running process has requested a blocking operation (goes to
   blocked queue for 5 units of time)

   SIGHUP - re-read the config file and update the program's behaviour
   based on the new config file contents

3. CTRL-C sends a SIGINT to the program which frees the memory and exits the
   program.

4. Modifying the config file determins how often the SIGALRM runs.



Configuration
-------------

Edit the config.txt file and change the timer=x, where x is the seconds for a
unit of time, to however long you want a unit of time to be for your processes.


Method Descriptions
------------------

list.c and list.h - Linked List Structure
-----------------

Linked list structure with methods for adding, removing, and getting
nodes from a list.

parse.c and parse.h - Main File
-------------------

This does all of the logic, intterupt handling and gets user input for the
program.

State functions
---------------

void readyState();
	- Handles logic for moving from the ready state to other states.
void runningState();
	- Handles logic for moving from the running state to other states.
void blockedState();
	- Handles logic for moving from the blocked state to other states.
void stateTransitions(int the_signal);
	- Handles logic for what to run every time a SIGALRM inttterupt happens.

Intterupt Handlers and Functions
--------------------------------

void moveToBlockedState(int signal);
void cleanupAndExit(int signal);
int setUpStateLister();
int setUpBlocker();
int setUpConfigUpdater();
int setUpExit();

Queue display
-------------

void printQueue(List_t *list, char *name);
void displayQueueInfo(int signal);
void updateQueueTime(List_t *list);

Config functions
----------------

int readConfigTimer();
int setUpAlarm(int timer);
void updateConfigTimer(int signal);


Citations
=========

Travis Roberts helped explain how the linked list functions worked with the void
pointers, and how you can pop off the head and add to another queue without
segmentation faults or errors.






