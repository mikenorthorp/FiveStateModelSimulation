/* parse.c
   Michael Northorp - CSCI 3120 - Assignment 1
   This program simulates processes going through the
   5 state model for process management.
   Processes start in a new queue when they are created,
   and they travel too the ready queue right away. They then
   go into the running queue if possible and run for their
   running duration. Processes can be blocked by system intterupts
   and will go into the exit queue when they have ran for their lifetime
   duration. For more details see the readme. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> //For interrupt handling
#include <unistd.h> //For SIGALRM function
#include "list.h"
#include "parse.h"

/* Global variables for timer and queues */
int timer = 0;
List_t processes;
List_t running;
List_t blocked;

/***********************************
 *  Main
 ***********************************/
int
main( int argc, char **argv )
{
    // Declare initial variables
    int    error_code = 0;  /* In unix, a program returns 0 if all is ok. */
    char   inputbuffer[BUF_MAX];
    char   name[BUF_MAX];
    int    lifetime;
    int    runningTime;
    pcb_t  *pcb;

    /* Initialize my data structures. */
    if (List_init( &processes ) && List_init( &running ) && List_init( &blocked ))
    {
        /* Set up and initialization */

        // Initialize timer value
        timer = readConfigTimer();

        //Set up handlers
        setUpAlarm(timer);
        setUpExit();
        setUpBlocker();
        setUpConfigUpdater();
        setUpStateLister();

        /* End set up */

        /* This creates an infinite loop from which you will need to do a
           ^C to stop the program.  This may not be the most elegant solution
           for you. */
        while (1)
        {
            if (fgets( inputbuffer, BUF_MAX - 1, stdin ))
            {
                /* Put the parameters into a PCB and store it in the list of processes. */
                if (sscanf( inputbuffer, "%s %d %d", name, &lifetime, &runningTime) == 3)
                {
                    /* We have all the input that is required. */

                    // Allocate memory for PCB
                    pcb = (pcb_t *) malloc( sizeof( pcb_t ) );
                    if (pcb != NULL)
                    {
                        strncpy( pcb->name, name, NAME_MAX - 1 );
                        pcb->name[NAME_MAX] = '\0'; /* Make sure that it is null-terminated. */
                        pcb->lifetime = lifetime;
                        pcb->runningTime = runningTime;
                        pcb->time_in_state = 0;

                        //Show that process has been read and stored
                        printf ("\nRead and stored process %s with lifetime %d and running time of %d\n", pcb->name, pcb->lifetime, pcb->runningTime);

                        // Could not insert process into list
                        if (List_add_tail( &processes, (void *)pcb ) == 0)
                        {
                            printf ("Error in inserting the process into the list.\n");
                        }

                        // Error, not enough memory to make PCB
                    }
                    else
                    {
                        printf( "Unable to allocate enough memory.\n");
                    }

                    // Incorrect number of parameters read in
                }
                else
                {
                    printf ("Incorrect number of parameters read.\n");
                }

                // No paramters read in
            }
            else
            {
                printf ("Nothing read in.\n");
            }
        }

    }
    else
    {
        printf ("Unable to initialize the list of processes.\n");
        error_code = 1;
    }

    // End of program
    return error_code;
}


/***********************************
 *  Main functions
 ***********************************/

/***********************************
 *  State functions
 ***********************************/

/* Moves the head node from ready to the running queue */
void
readyState()
{
    /* Call move code if there is something at the head of the ready queue and
       there is no running process */
    if (processes.head != NULL && running.head == NULL)
    {
        // Create pcb_t pointer for process and state transitions
        pcb_t *process;

        // Remove from ready queue
        List_remove_head(&processes, (void *)&process);

        // Add to running queue
        List_add_head(&running, process);

        // Print out transition
        printf("\n%s transition from ready (%d) to running state\n", process->name, process->time_in_state);

        process->time_in_state = 0;
    }
    else
    {
        // Ready queue is empty so do nothing
    }
}

/* Moves the head node from the running queue to the ready queue */
void
runningState()
{
    /* Call move code if there is something at the head of the running queue */
    if (running.head != NULL)
    {

        // Create pcb_t pointer for process and state transitions
        pcb_t *process;
        // Get head of running processes
        List_head_info(&running, (void *)&process);

        process->lifetime--;
        // Send to Exit queue and deallocate when lifetime is 0 or running time is set to 0
        if (process->lifetime <= 0 || process->runningTime <= 0)
        {
            // Remove and deallocate
            List_remove_head(&running, (void *)&process);

            // Print out transition
            printf("\n%s transition from running (%d) to exit state\n", process->name, process->time_in_state);

            // Free the memory up
            free(process);
        }

        // Time on CPU has expired for current running process
        else if (process->time_in_state >= process->runningTime && process->lifetime > 0)
        {
            // Remove from running queue
            List_remove_head(&running, (void *)&process);

            // Add to ready queue tail
            List_add_tail(&processes, process);

            // Print out transition
            printf("\n%s transition from running (%d) to ready state\n", process->name, process->time_in_state);

            //Reset time in state
            process->time_in_state = 0;
        }
    }
}

/* Checks if anything is in blocked state and increases time in head timer until
   it hits 5, then removes it from blocked state and adds to ready queue */
void
blockedState()
{
    // Keep track of time in state
    static int time_at_head = 0;

    /* Call move code if there is something at the head of the blocked queue */
    if (blocked.head != NULL)
    {
        time_at_head++;
        // Processe has been in blocked state for 5 time units
        if (time_at_head == 5)
        {
            // Create pcb_t pointer for process and state transitions
            pcb_t *process;

            // Remove from blocked queue
            List_remove_head(&blocked, (void *)&process);

            // Add to ready queue tail
            List_add_tail(&processes, process);

            // Print out transition
            printf("\n%s transition from blocked (%d) to ready state\n", process->name, process->time_in_state);

            //Reset time in state
            time_at_head = 0;
            process->time_in_state = 0;
        }
    }
    else
    {
        // Nothing in queue so do nothing, no processes blocked
    }
}

/* This function overides the default SIGALRM and handles all of the state
   transitions for the different processes whenever the SIGALRM happens */
void
stateTransitions( int the_signal )
{
    // Update all queues by one time unit for their time in state
    updateQueueTime(&processes);
    updateQueueTime(&running);
    updateQueueTime(&blocked);

    // Move anything in running to ready if possible
    runningState();
    // Move anything in ready to running if possible
    readyState();
    // Move anything from blocked to ready if possible
    blockedState();

    // Reset timer
    alarm( timer );
}

/***********************************
 *  Intterupt Handlers and Functions
 ***********************************/

/* Moves the head node from the running queue to the blocked queue*/
void
moveToBlockedState(int signal)
{
    /* Call code to move from running state to blocked if something is in the running
       state */
    if (running.head != NULL)
    {
        // Create pcb_t pointer for process and state transitions
        pcb_t *process;

        // Remove from running queue
        List_remove_head(&running, (void *)&process);

        // Add to blocked queue
        List_add_tail(&blocked, process);

        // Print out transition from running to blocked state
        printf("\n%s transition from running (%d) to blocked state\n", process->name, process->time_in_state);

        // Reset timer
        process->time_in_state = 0;
    }
}

/* This frees up all of the list nodes and the node data */
void
cleanupAndExit(int signal)
{
    printf("\nFreeing up memory\n");

    /* Free up lists (modified List_destroy function to remove pcb_t data as well
       as node itself) */
    List_destroy(&processes);
    List_destroy(&running);
    List_destroy(&blocked);

    // Exit program
    printf("Exiting program...\n");
    exit(0);
}

/* Set up SIGUSR1 intterupt handler to print the state of the queues,
   ready, running and blocked */
int
setUpStateLister()
{
    int return_code = 0;

    if (signal( SIGUSR1, displayQueueInfo ) == SIG_ERR)
    {
        printf ("Unable to install handler\n");
        return_code = 1;
    }

    return return_code;
}

/* Set up SIGUSR2 intterupt handler to block the running process  */
int
setUpBlocker()
{
    int return_code = 0;

    if (signal( SIGUSR2, moveToBlockedState ) == SIG_ERR)
    {
        printf ("Unable to install handler\n");
        return_code = 1;
    }

    return return_code;
}

/* Set up SIGHUP intterupt handler
   to reread the config file and update the alarm timer */
int
setUpConfigUpdater()
{
    int return_code = 0;

    if (signal( SIGHUP, updateConfigTimer ) == SIG_ERR)
    {
        printf ("Unable to install handler\n");
        return_code = 1;
    }

    return return_code;
}
/* Set up SIGINT intterupt handler */
int
setUpExit()
{
    int return_code = 0;

    if (signal( SIGINT, cleanupAndExit ) == SIG_ERR)
    {
        printf ("Unable to install handler\n");
        return_code = 1;
    }

    return return_code;
}


/***********************************
 *  Queue display
 ***********************************/

/* This prints out the queue info for a single queue */
void
printQueue(List_t *list, char *name)
{
    // Create current node and last node pointers
    pcb_t *currentNode = NULL;
    pcb_t *lastNode = NULL;

    // Print name of queue at top
    printf("%s queue:\n", name);

    // If the head is not null
    if ((list != NULL) && (list->head != NULL))
    {
        // Get the head node of the list
        List_next_node(list, (void *)&lastNode, (void *)&currentNode);
        // Loop through the list until null
        while (currentNode != NULL)
        {
            // Print out current node info, and get next node in list
            printf("\t%s %d\n", currentNode->name, currentNode->lifetime);
            List_next_node(list, (void *)&lastNode, (void *)&currentNode);
        }
    }
    // Print empty if nothing in list
    else
    {
        printf("\tEmpty");
    }
}

/* This displays the queue info for all of the queues */
void
displayQueueInfo(int signal)
{
    // Print out ready queue
    printf("\n");
    printQueue(&processes, "Ready");
    printf("\n");
    // Print out running queue
    printQueue(&running, "Running");
    printf("\n");
    // Print out blocked queue
    printQueue(&blocked, "Blocked");
    printf("\n");
}

/* This functions goes through a queue and updates the time each pcb_t has
   been in their state by one */
void
updateQueueTime(List_t *list)
{
    // Create current node and last node pointers
    pcb_t *currentNode = NULL;
    pcb_t *lastNode = NULL;

    // If the head is not null
    if ((list != NULL) && (list->head != NULL))
    {
        // Get head node
        List_next_node(list, (void *)&lastNode, (void *)&currentNode);
        // Loop while node is not null
        while (currentNode != NULL)
        {
            // Increase the time the node was in its current state
            currentNode->time_in_state++;
            List_next_node(list, (void *)&lastNode, (void *)&currentNode);
        }
    }
}


/***********************************
 *  Config functions
 ***********************************/

/* This reads in the timer for the unit of time from
   the config.txt file */
int
readConfigTimer()
{
    FILE *inFile;
    int unitOfTime = 0;

    // Open file
    inFile = fopen("config.txt", "r");

    // Check that file exits, return 0 if not
    if (inFile == NULL)
    {
        printf("Error: File not found\n");
        return unitOfTime;
    }

    // Scan in the unit of time in the format timer=[num]
    fscanf(inFile, "timer=%d", &unitOfTime);
    printf("The timer is %d second(s) per unit of time\n\n", unitOfTime);

    // Close the file
    fclose(inFile);

    // Return the timer value
    return unitOfTime;
}

/* Set up SIGALRM with custom timer from config.txt */
int
setUpAlarm(int timer)
{
    int return_code = 0;

    // Set up intterupt handler
    if (signal( SIGALRM, stateTransitions ) == SIG_ERR)
    {
        printf ("Unable to install handler\n");
        return_code = 1;
    }
    else
    {
        /* Start the timer and then wait! */
        alarm( timer );
    }

    return return_code;
}

/* Updates config timer when called by SIGHUP intterupt*/
void
updateConfigTimer(int signal)
{
    timer = readConfigTimer();
}

