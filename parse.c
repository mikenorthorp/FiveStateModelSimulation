#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> //For interrupt handling
#include "list.h"


#define BUF_MAX  (81)
#define NAME_MAX (21)

// Global variables for timer and queues
int timer = 0;
List_t processes;
List_t running;
List_t blocked;

/* This function overides the default SIGALRM and handles all of the state
   transitions for the different processes */
void
stateTransitions( int the_signal )
{
    printf ("One time unit has passed\n");

    alarm( timer );
}


/* Set up SIGALRM with custom timer from config.txt */
int
setUpAlarm(int timer)
{
    int return_code = 0;

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

/* This reads in the timer for the unit of time from
   the config.txt file */
int
readConfigTimer()
{
    FILE *inFile;
    int unitOfTime = 0;

    // Open file
    inFile = fopen("config.txt", "r");

    // Check that file exits
    if (inFile == NULL)
    {
        printf("Error: File not found\n");
        return unitOfTime;
    }

    // Scan in the unit of time in the format timer=[num]
    fscanf(inFile, "timer=%d", &unitOfTime);
    printf("The timer is %d", unitOfTime);

    return unitOfTime;
}

/* Define the struct for the PCB
   Takes in name, lifetime and running time */
typedef struct
{
    char name[NAME_MAX];
    int  lifetime;
    int  runningTime;
} pcb_t;

// Main function
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

        timer = readConfigTimer();
        //Set up alarm code
        setUpAlarm(timer);

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

                        //Show that process has been read and stored
                        printf ("Read and stored process %s with lifetime %d and running time of %d\n", pcb->name, pcb->lifetime, pcb->runningTime);

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

